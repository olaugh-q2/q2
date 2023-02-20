#include "src/scrabble/move_finder.h"

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

std::unique_ptr<Tiles> tiles_;
std::unique_ptr<AnagramMap> anagram_map_;
std::unique_ptr<BoardLayout> board_layout_;
std::unique_ptr<MoveFinder> move_finder_;

class MoveFinderTest : public testing::Test {
 protected:
  static void SetUpTestSuite() {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    LOG(INFO) << "tiles ok";
    anagram_map_ = AnagramMap::CreateFromBinaryFile(
        *tiles_, "src/scrabble/testdata/csw21.qam");
    LOG(INFO) << "anagram map ok";
    board_layout_ = absl::make_unique<BoardLayout>(
        "src/scrabble/testdata/scrabble_board.textproto");
    LOG(INFO) << "board layout ok";
    move_finder_ =
        absl::make_unique<MoveFinder>(*anagram_map_, *board_layout_, *tiles_);
    LOG(INFO) << "move finder ok";
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }
  void ExpectMoves(const std::vector<Move>& moves,
                   const std::vector<std::string>& expected) {
    std::vector<std::string> actual;
    for (const auto& move : moves) {
      std::stringstream ss;
      move.Display(*tiles_, ss);
      actual.push_back(ss.str());
    }
    EXPECT_THAT(actual, testing::UnorderedElementsAreArray(expected));
  }
};

TEST_F(MoveFinderTest, Blankify) {
  const LetterString stain = tiles_->ToLetterString("STAIN").value();
  const LetterString strain = tiles_->ToLetterString("STRAIN").value();
  const LetterString tannins = tiles_->ToLetterString("TANNINS").value();
  const LetterString tanist = tiles_->ToLetterString("TANIST").value();
  const LetterString tanists = tiles_->ToLetterString("TANISTS").value();
  const LetterString inkstains = tiles_->ToLetterString("...STAINS").value();

  EXPECT_THAT(move_finder_->Blankify(stain, strain), ElementsAre(LS("STrAIN")));
  EXPECT_THAT(
      move_finder_->Blankify(stain, tannins),
      UnorderedElementsAre(LS("TANnInS"), LS("TAnNInS"), LS("TAnnINS")));
  EXPECT_THAT(move_finder_->Blankify(stain, tanist),
              UnorderedElementsAre(LS("TANISt"), LS("tANIST")));
  EXPECT_THAT(move_finder_->Blankify(stain, tanists),
              UnorderedElementsAre(LS("TANISts"), LS("TANIstS"), LS("tANISTs"),
                                   LS("tANIsTS")));
  EXPECT_THAT(move_finder_->Blankify(stain, inkstains),
              UnorderedElementsAre(LS("...sTAINS"), LS("...STAINs")));
}

TEST_F(MoveFinderTest, FindExchanges) {
  const Rack rack(tiles_->ToLetterString("AB").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH B", "EXCH AB"});
}

TEST_F(MoveFinderTest, FindExchanges2) {
  const Rack rack(tiles_->ToLetterString("AAA").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH AA", "EXCH AAA"});
}

TEST_F(MoveFinderTest, FindExchanges3) {
  const Rack rack(tiles_->ToLetterString("A?").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH ?", "EXCH A?"});
}

TEST_F(MoveFinderTest, FindWords) {
  Board board;
  const Rack rack(tiles_->ToLetterString("IIICFVV").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 7, 7);
  ExpectMoves(words, {"8H VIVIFIC (score = 94)"});
}

TEST_F(MoveFinderTest, FindWords2) {
  Board board;
  const Rack rack(tiles_->ToLetterString("BANANAS").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 6, 2);
  ExpectMoves(words,
              {"8G AA (score = 4)", "8G AB (score = 8)", "8G BA (score = 8)",
               "8G AN (score = 4)", "8G NA (score = 4)", "8G AS (score = 4)"});
}

TEST_F(MoveFinderTest, FindWords3) {
  Board board;
  const Rack rack(tiles_->ToLetterString("AEURVY?").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 5, 7);
  ExpectMoves(words, {"8F qUAVERY (score = 82)"});
}

TEST_F(MoveFinderTest, FindWords4) {
  Board board;
  const Rack rack(tiles_->ToLetterString("SQIRT??").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 1, 7);
  ExpectMoves(words, {"8B QInTaRS (score = 78)", "8B QueRIST (score = 78)",
                      "8B ReQuITS (score = 98)", "8B SQuIRTs (score = 78)",
                      "8B sQuIRTS (score = 78)"});
}

TEST_F(MoveFinderTest, ZeroPlayedThroughTiles) {
  Board board;
  const auto yag = Move::Parse("8G YAG", *tiles_);
  board.UnsafePlaceMove(yag.value());
  const auto before = tiles_->ToLetterString("SATYAGRAHA").value();
  const auto after =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Across, 7, 3, before);
  EXPECT_TRUE(after.has_value());
  EXPECT_EQ(*after, tiles_->ToLetterString("SAT...RAHA").value());

  const auto after2 =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Across, 7, 4, before);
  EXPECT_FALSE(after2.has_value());
}

TEST_F(MoveFinderTest, AbsorbThroughTiles) {
  Board board;
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 6, 3), 1);

  const auto but = Move::Parse("8H BuT", *tiles_);
  board.UnsafePlaceMove(but.value());
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 7, 3),
            tiles_->ToProduct(tiles_->ToLetterString("BUT").value()));
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 5, 3),
            tiles_->ToProduct(tiles_->ToLetterString("BUT").value()));

  const auto zone = Move::Parse("8L ZONE", *tiles_);
  board.UnsafePlaceMove(zone.value());
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 6, 3),
            tiles_->ToProduct(tiles_->ToLetterString("BUTZONE").value()));

  const auto oxy = Move::Parse("8A OXY", *tiles_);
  board.UnsafePlaceMove(oxy.value());
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 0, 5),
            tiles_->ToProduct(tiles_->ToLetterString("OXYBUTZONE").value()));
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 0, 1),
            tiles_->ToProduct(tiles_->ToLetterString("OXY").value()));
}

TEST_F(MoveFinderTest, AbsorbThroughTiles2) {
  Board board;
  const auto but = Move::Parse("8H BUT", *tiles_);
  board.UnsafePlaceMove(but.value());
  const auto zo = Move::Parse("8L ZO", *tiles_);
  board.UnsafePlaceMove(zo.value());
  const auto he = Move::Parse("8E HE", *tiles_);
  board.UnsafePlaceMove(he.value());
  const auto ixnay = Move::Parse("B7 IxNAY", *tiles_);
  board.UnsafePlaceMove(ixnay.value());
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 0, 7),
            tiles_->ToProduct(tiles_->ToLetterString("XHEBUTZO").value()));
  EXPECT_EQ(move_finder_->AbsorbThroughTiles(board, Move::Across, 7, 0, 1),
            tiles_->ToProduct(tiles_->ToLetterString("X").value()));
}

TEST_F(MoveFinderTest, ZeroPlayedThroughTilesVertical) {
  Board board;
  const auto yag = Move::Parse("H7 YAG", *tiles_);
  board.UnsafePlaceMove(yag.value());
  const auto before = tiles_->ToLetterString("SATYAGRAHA").value();
  const auto after =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Down, 3, 7, before);
  EXPECT_TRUE(after.has_value());
  EXPECT_EQ(*after, tiles_->ToLetterString("SAT...RAHA").value());

  const auto after2 =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Down, 4, 7, before);
  EXPECT_FALSE(after2.has_value());
}

TEST_F(MoveFinderTest, PlayThroughWithBlanks) {
  Board board;
  const auto con = Move::Parse("8G CON", *tiles_);
  board.UnsafePlaceMove(con.value());
  const Rack rack(tiles_->ToLetterString("URATES?").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 4, 7);
  ExpectMoves(words,
              {"8E RA...TEUrS (score = 62)", "8E rA...TEURS (score = 62)",
               "8E RA...TEUSe (score = 62)", "8E RA...TeUSE (score = 62)"});
}

TEST_F(MoveFinderTest, CrossAt) {
  Board board;
  const auto ky = Move::Parse("8G KY", *tiles_);
  board.UnsafePlaceMove(ky.value());
  const auto cross_7G = move_finder_->CrossAt(board, Move::Across, 6, 6, true);
  ASSERT_TRUE(cross_7G.has_value());
  EXPECT_EQ(*cross_7G, LS(".K"));

  const auto oo = Move::Parse("9G oo", *tiles_);
  board.UnsafePlaceMove(oo.value());
  const auto cross_7H = move_finder_->CrossAt(board, Move::Across, 6, 7, false);
  ASSERT_TRUE(cross_7H.has_value());
  EXPECT_EQ(*cross_7H, LS(".Y?"));
}

TEST_F(MoveFinderTest, HookSum) {
  Board board;
  const auto ky = Move::Parse("8G KY", *tiles_);
  board.UnsafePlaceMove(ky.value());
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 6, 6, 3), 9);

  const auto oo = Move::Parse("9G oo", *tiles_);
  board.UnsafePlaceMove(oo.value());
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 6, 6, 3), 9);

  const auto beau = Move::Parse("10E BEAU", *tiles_);
  board.UnsafePlaceMove(beau.value());
  // 6 (Bx2) + 1 (E) + 6 (KoA) + 5 (YoU) = 19
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 10, 0, 15), 18);
}

TEST_F(MoveFinderTest, CheckHooks) {
  Board board;
  const auto aa = Move::Parse("8G AA", *tiles_);
  board.UnsafePlaceMove(aa.value());

  const auto hm = Move::Parse("7G HM", *tiles_);
  EXPECT_TRUE(move_finder_->CheckHooks(board, hm.value()));

  const auto ae = Move::Parse("7G AE", *tiles_);
  EXPECT_TRUE(move_finder_->CheckHooks(board, ae.value()));

  const auto faan = Move::Parse("7F FAAN", *tiles_);
  EXPECT_TRUE(move_finder_->CheckHooks(board, faan.value()));

  const auto qi = Move::Parse("7G QI", *tiles_);
  EXPECT_FALSE(move_finder_->CheckHooks(board, qi.value()));

  const auto ax = Move::Parse("7G AX", *tiles_);
  EXPECT_FALSE(move_finder_->CheckHooks(board, ax.value()));
}

TEST_F(MoveFinderTest, SevenTileOverlap) {
  Board board;
  const auto airtime = Move::Parse("8G AiRTIME", *tiles_);
  board.UnsafePlaceMove(airtime.value());

  const Rack rack(tiles_->ToLetterString("HEATER?").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 6, 6, 7);
  ExpectMoves(words, {"7G tHEATER (score = 80)", "7G THEAtER (score = 82)"});
}

TEST_F(MoveFinderTest, NonHooks) {
  Board board;
  const auto vav = Move::Parse("8G VAV", *tiles_);
  board.UnsafePlaceMove(vav.value());

  const Rack rack(tiles_->ToLetterString("Z??").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 6, 6, 3);
  ExpectMoves(words, {});
}

TEST_F(MoveFinderTest, FrontExtension) {
  Board board;
  const auto airtime = Move::Parse("8H DEMANDS", *tiles_);
  board.UnsafePlaceMove(airtime.value());

  const Rack rack(tiles_->ToLetterString("??UNTER").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 0, 7);
  ExpectMoves(words, {"8A coUNTER....... (score = 101)"});
}

TEST_F(MoveFinderTest, EmptyBoardSpots) {
  Board board;
  const Rack rack(tiles_->ToLetterString("LULZ").value());
  const auto spots4 = move_finder_->FindSpots(rack, board);
  EXPECT_THAT(spots4,
              UnorderedElementsAre(MoveFinder::Spot(Move::Across, 7, 6, 2),
                                   MoveFinder::Spot(Move::Across, 7, 7, 2),
                                   MoveFinder::Spot(Move::Across, 7, 5, 3),
                                   MoveFinder::Spot(Move::Across, 7, 6, 3),
                                   MoveFinder::Spot(Move::Across, 7, 7, 3),
                                   MoveFinder::Spot(Move::Across, 7, 4, 4),
                                   MoveFinder::Spot(Move::Across, 7, 5, 4),
                                   MoveFinder::Spot(Move::Across, 7, 6, 4),
                                   MoveFinder::Spot(Move::Across, 7, 7, 4)));
}

TEST_F(MoveFinderTest, AcrossSpots) {
  Board board;
  const auto an = Move::Parse("8H AN", *tiles_);
  board.UnsafePlaceMove(an.value());
  const auto ax = Move::Parse("9G AX", *tiles_);
  board.UnsafePlaceMove(ax.value());
  /*
    ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
   1＝　　＇　　　＝　　　＇　　＝
   2　－　　　＂　　　＂　　　－　
   3　　－　　　＇　＇　　　－　　
   4＇　　－　　　＇　　　－　　＇
   5　　　　－　　　　　－　　　　
   6　＂　　　＂　　　＂　　　＂　
   7　　＇　　　＇　＇　　　＇　　
   8＝　　＇　　　ＡＮ　　＇　　＝
   9　　＇　　　ＡＸ＇　　　＇　　
  10　＂　　　＂　　　＂　　　＂　
  11　　　　－　　　　　－　　　　
  12＇　　－　　　＇　　　－　　＇
  13　　－　　　＇　＇　　　－　　
  14　－　　　＂　　　＂　　　－　
  15＝　　＇　　　＝　　　＇　　＝
  */
  std::vector<MoveFinder::Spot> spots;
  move_finder_->FindSpots(3, board, Move::Across, &spots);
  EXPECT_THAT(spots, UnorderedElementsAreArray(
                         {MoveFinder::Spot(Move::Across, 6, 5, 3),
                          MoveFinder::Spot(Move::Across, 6, 6, 2),
                          MoveFinder::Spot(Move::Across, 6, 6, 3),
                          MoveFinder::Spot(Move::Across, 6, 7, 2),
                          MoveFinder::Spot(Move::Across, 6, 7, 3),
                          MoveFinder::Spot(Move::Across, 6, 8, 2),
                          MoveFinder::Spot(Move::Across, 6, 8, 3),
                          MoveFinder::Spot(Move::Across, 7, 4, 3),
                          MoveFinder::Spot(Move::Across, 7, 5, 2),
                          MoveFinder::Spot(Move::Across, 7, 5, 3),
                          MoveFinder::Spot(Move::Across, 7, 6, 1),
                          MoveFinder::Spot(Move::Across, 7, 6, 2),
                          MoveFinder::Spot(Move::Across, 7, 6, 3),
                          MoveFinder::Spot(Move::Across, 7, 7, 1),
                          MoveFinder::Spot(Move::Across, 7, 7, 2),
                          MoveFinder::Spot(Move::Across, 7, 7, 3),
                          MoveFinder::Spot(Move::Across, 8, 3, 3),
                          MoveFinder::Spot(Move::Across, 8, 4, 2),
                          MoveFinder::Spot(Move::Across, 8, 4, 3),
                          MoveFinder::Spot(Move::Across, 8, 5, 1),
                          MoveFinder::Spot(Move::Across, 8, 5, 2),
                          MoveFinder::Spot(Move::Across, 8, 5, 3),
                          MoveFinder::Spot(Move::Across, 8, 6, 1),
                          MoveFinder::Spot(Move::Across, 8, 6, 2),
                          MoveFinder::Spot(Move::Across, 8, 6, 3),
                          MoveFinder::Spot(Move::Across, 9, 4, 3),
                          MoveFinder::Spot(Move::Across, 9, 5, 2),
                          MoveFinder::Spot(Move::Across, 9, 5, 3),
                          MoveFinder::Spot(Move::Across, 9, 6, 2),
                          MoveFinder::Spot(Move::Across, 9, 6, 3),
                          MoveFinder::Spot(Move::Across, 9, 7, 2),
                          MoveFinder::Spot(Move::Across, 9, 7, 3)}));

  spots.clear();
  const auto am = Move::Parse("G9 .M", *tiles_);
  board.UnsafePlaceMove(am.value());
  /*
    ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
   1＝　　＇　　　＝　　　＇　　＝
   2　－　　　＂　　　＂　　　－　
   3　　－　　　＇　＇　　　－　　
   4＇　　－　　　＇　　　－　　＇
   5　　　　－　　　　　－　　　　
   6　＂　　　＂　　　＂　　　＂　
   7　　＇　　　＇　＇　　　＇　　
   8＝　　＇　　　ＡＮ　　＇　　＝
   9　　＇　　　ＡＸ＇　　　＇　　
  10　＂　　　＂Ｍ　　＂　　　＂　
  11　　　　－　　　　　－　　　　
  12＇　　－　　　＇　　　－　　＇
  13　　－　　　＇　＇　　　－　　
  14　－　　　＂　　　＂　　　－　
  15＝　　＇　　　＝　　　＇　　＝
  */
  move_finder_->FindSpots(1, board, Move::Across, &spots);
  EXPECT_THAT(spots, UnorderedElementsAreArray(
                         {MoveFinder::Spot(Move::Across, 7, 6, 1),
                          MoveFinder::Spot(Move::Across, 7, 7, 1),
                          MoveFinder::Spot(Move::Across, 8, 5, 1),
                          MoveFinder::Spot(Move::Across, 8, 6, 1),
                          MoveFinder::Spot(Move::Across, 9, 5, 1),
                          MoveFinder::Spot(Move::Down, 7, 7, 1)}));

  spots.clear();
  const auto amp = Move::Parse("G9 .MP", *tiles_);
  board.UnsafePlaceMove(amp.value());
  /*
    ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
   1＝　　＇　　　＝　　　＇　　＝
   2　－　　　＂　　　＂　　　－　
   3　　－　　　＇　＇　　　－　　
   4＇　　－　　　＇　　　－　　＇
   5　　　　－　　　　　－　　　　
   6　＂　　　＂　　　＂　　　＂　
   7　　＇　　　＇　＇　　　＇　　
   8＝　　＇　　　ＡＮ　　＇　　＝
   9　　＇　　　ＡＸ＇　　　＇　　
  10　＂　　　＂Ｍ　　＂　　　＂　
  11　　　　－　Ｐ　　　－　　　　
  12＇　　－　　　＇　　　－　　＇
  13　　－　　　＇　＇　　　－　　
  14　－　　　＂　　　＂　　　－　
  15＝　　＇　　　＝　　　＇　　＝
  */
  move_finder_->FindSpots(1, board, Move::Across, &spots);
  EXPECT_THAT(spots, UnorderedElementsAreArray(
                         {MoveFinder::Spot(Move::Down, 7, 6, 1),
                          MoveFinder::Spot(Move::Across, 7, 7, 1),
                          MoveFinder::Spot(Move::Across, 8, 5, 1),
                          MoveFinder::Spot(Move::Across, 8, 6, 1),
                          MoveFinder::Spot(Move::Across, 9, 5, 1),
                          MoveFinder::Spot(Move::Down, 7, 7, 1),
                          MoveFinder::Spot(Move::Across, 10, 5, 1),
                          MoveFinder::Spot(Move::Across, 10, 6, 1)}));
}

TEST_F(MoveFinderTest, DownSpots) {
  Board board;
  const auto an = Move::Parse("8H AN", *tiles_);
  board.UnsafePlaceMove(an.value());
  const auto ax = Move::Parse("9G AX", *tiles_);
  board.UnsafePlaceMove(ax.value());
  /*
    ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
   1＝　　＇　　　＝　　　＇　　＝
   2　－　　　＂　　　＂　　　－　
   3　　－　　　＇　＇　　　－　　
   4＇　　－　　　＇　　　－　　＇
   5　　　　－　　　　　－　　　　
   6　＂　　　＂　　　＂　　　＂　
   7　　＇　　　＇　＇　　　＇　　
   8＝　　＇　　　ＡＮ　　＇　　＝
   9　　＇　　　ＡＸ＇　　　＇　　
  10　＂　　　＂　　　＂　　　＂　
  11　　　　－　　　　　－　　　　
  12＇　　－　　　＇　　　－　　＇
  13　　－　　　＇　＇　　　－　　
  14　－　　　＂　　　＂　　　－　
  15＝　　＇　　　＝　　　＇　　＝
  */
  std::vector<MoveFinder::Spot> spots;
  move_finder_->FindSpots(3, board, Move::Down, &spots);
  EXPECT_THAT(spots, UnorderedElementsAreArray({
                         MoveFinder::Spot(Move::Down, 4, 7, 3),
                         MoveFinder::Spot(Move::Down, 4, 8, 3),
                         MoveFinder::Spot(Move::Down, 5, 6, 3),
                         MoveFinder::Spot(Move::Down, 5, 7, 2),
                         MoveFinder::Spot(Move::Down, 5, 7, 3),
                         MoveFinder::Spot(Move::Down, 5, 8, 2),
                         MoveFinder::Spot(Move::Down, 5, 8, 3),
                         MoveFinder::Spot(Move::Down, 5, 9, 3),
                         MoveFinder::Spot(Move::Down, 6, 5, 3),
                         MoveFinder::Spot(Move::Down, 6, 6, 2),
                         MoveFinder::Spot(Move::Down, 6, 6, 3),
                         MoveFinder::Spot(Move::Down, 6, 7, 1),
                         MoveFinder::Spot(Move::Down, 6, 7, 2),
                         MoveFinder::Spot(Move::Down, 6, 7, 3),
                         MoveFinder::Spot(Move::Down, 6, 8, 1),
                         MoveFinder::Spot(Move::Down, 6, 8, 2),
                         MoveFinder::Spot(Move::Down, 6, 8, 3),
                         MoveFinder::Spot(Move::Down, 6, 9, 2),
                         MoveFinder::Spot(Move::Down, 6, 9, 3),
                         MoveFinder::Spot(Move::Down, 7, 5, 2),
                         MoveFinder::Spot(Move::Down, 7, 5, 3),
                         MoveFinder::Spot(Move::Down, 7, 6, 2),
                         MoveFinder::Spot(Move::Down, 7, 6, 3),
                         MoveFinder::Spot(Move::Down, 7, 7, 1),
                         MoveFinder::Spot(Move::Down, 7, 7, 2),
                         MoveFinder::Spot(Move::Down, 7, 7, 3),
                         MoveFinder::Spot(Move::Down, 7, 8, 2),
                         MoveFinder::Spot(Move::Down, 7, 8, 3),
                         MoveFinder::Spot(Move::Down, 7, 9, 2),
                         MoveFinder::Spot(Move::Down, 7, 9, 3),
                         MoveFinder::Spot(Move::Down, 8, 5, 2),
                         MoveFinder::Spot(Move::Down, 8, 5, 3),
                         MoveFinder::Spot(Move::Down, 8, 6, 1),
                         MoveFinder::Spot(Move::Down, 8, 6, 2),
                         MoveFinder::Spot(Move::Down, 8, 6, 3),
                     }));

  spots.clear();
  const auto am = Move::Parse("G9 .M", *tiles_);
  board.UnsafePlaceMove(am.value());
  /*
    ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
   1＝　　＇　　　＝　　　＇　　＝
   2　－　　　＂　　　＂　　　－　
   3　　－　　　＇　＇　　　－　　
   4＇　　－　　　＇　　　－　　＇
   5　　　　－　　　　　－　　　　
   6　＂　　　＂　　　＂　　　＂　
   7　　＇　　　＇　＇　　　＇　　
   8＝　　＇　　　ＡＮ　　＇　　＝
   9　　＇　　　ＡＸ＇　　　＇　　
  10　＂　　　＂Ｍ　　＂　　　＂　
  11　　　　－　　　　　－　　　　
  12＇　　－　　　＇　　　－　　＇
  13　　－　　　＇　＇　　　－　　
  14　－　　　＂　　　＂　　　－　
  15＝　　＇　　　＝　　　＇　　＝
  */
  move_finder_->FindSpots(1, board, Move::Down, &spots);
  EXPECT_THAT(spots, UnorderedElementsAreArray(
                         {MoveFinder::Spot(Move::Down, 6, 7, 1),
                          MoveFinder::Spot(Move::Down, 6, 8, 1),
                          MoveFinder::Spot(Move::Down, 8, 6, 1)}));
}

TEST_F(MoveFinderTest, WordMultiplier) {
  Board board;
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Across, 7, 7, 5), 2);
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Across, 7, 7, 8), 6);
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Across, 7, 0, 15), 18);
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Across, 0, 0, 15), 27);
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Across, 0, 0, 14), 9);
  EXPECT_EQ(move_finder_->WordMultiplier(board, Move::Down, 0, 0, 15), 27);
}

TEST_F(MoveFinderTest, ThroughScore) {
  Board board;
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 6, 3), 0);

  const auto but = Move::Parse("8H BuT", *tiles_);
  board.UnsafePlaceMove(but.value());
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 6, 3),
            tiles_->Score(LS("B?T")));
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 5, 3),
            tiles_->Score(LS("B?T")));

  const auto zone = Move::Parse("8L ZONE", *tiles_);
  board.UnsafePlaceMove(zone.value());
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 6, 3),
            tiles_->Score(LS("B?TZONE")));

  const auto oxy = Move::Parse("8A OXY", *tiles_);
  board.UnsafePlaceMove(oxy.value());
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 0, 5),
            tiles_->Score(LS("OXYB?TZONE")));
  EXPECT_EQ(move_finder_->ThroughScore(board, Move::Across, 7, 0, 1),
            tiles_->Score(LS("OXY")));
}

TEST_F(MoveFinderTest, WordScore) {
  Board board;
  const auto cwm = Move::Parse("8G CWM", *tiles_);
  EXPECT_EQ(move_finder_->WordScore(board, cwm.value(), 1), 10);

  board.UnsafePlaceMove(cwm.value());
  const auto euoi = Move::Parse("7H EUOI", *tiles_);
  // 2 + 4 + 1 + 1 = 7
  EXPECT_EQ(move_finder_->WordScore(board, euoi.value(), 1), 8);

  board.UnsafePlaceMove(euoi.value());
  const auto zol = Move::Parse("6J ZOL", *tiles_);
  // 60 + 2 + 1 = 63
  EXPECT_EQ(move_finder_->WordScore(board, zol.value(), 1), 63);

  board.UnsafePlaceMove(zol.value());
  const auto aka = Move::Parse("5J AKA", *tiles_);
  // 7*2 + 1 + 5*2 + 1 = 26
  EXPECT_EQ(move_finder_->WordScore(board, aka.value(), 2), 26);
}

TEST_F(MoveFinderTest, FindMoves) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QUACKLE").value());
  std::vector<Move> moves = move_finder_->FindMoves(rack, board);
  // for (const auto& move : moves) {
  //   std::stringstream ss;
  //   move.Display(*tiles_, ss);
  //   LOG(INFO) << ss.str();
  // }
  //  TODO: These are just the scoring plays, no exchanges yet.
  int num_expected = 1 * 7 + 2 * 6 + 6 * 5 + 17 * 4 + 23 * 3 + 6 * 2;
  EXPECT_EQ(moves.size(), num_expected);
}