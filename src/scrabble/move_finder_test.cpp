#include "src/scrabble/move_finder.h"

#include <algorithm>
#include <tuple>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/move.h"

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

std::unique_ptr<Bag> full_bag_;
std::unique_ptr<Bag> empty_bag_;
std::unique_ptr<Tiles> tiles_;
std::unique_ptr<Leaves> leaves_;
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
    leaves_ = Leaves::CreateFromBinaryFile(
        *tiles_, "src/scrabble/testdata/csw_scrabble_macondo.qlv");
    move_finder_ = absl::make_unique<MoveFinder>(*anagram_map_, *board_layout_,
                                                 *tiles_, *leaves_);
    LOG(INFO) << "move finder ok";

    full_bag_ = absl::make_unique<Bag>(*tiles_);
    const std::vector<Letter> empty;
    empty_bag_ = absl::make_unique<Bag>(*tiles_, empty);
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }
  uint32_t H(const std::string& s) {
    uint32_t ret = 0;
    for (const Letter letter : LS(s)) {
      ret |= 1 << letter;
    }
    return ret;
  }
  int Score(const std::string& s) {
    int ret = 0;
    for (const Letter letter : LS(s)) {
      ret += tiles_->Score(letter);
    }
    return ret;
  }

  void ExpectMove(const Move& move, const std::string& expected) {
    std::stringstream ss;
    move.Display(*tiles_, ss);
    const auto actual = ss.str();
    EXPECT_EQ(actual, expected);
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

TEST_F(MoveFinderTest, BlankifyAllBlanks) {
  const LetterString empty;
  const LetterString aa = tiles_->ToLetterString("AA").value();

  EXPECT_THAT(move_finder_->Blankify(empty, aa), ElementsAre(LS("aa")));
}

TEST_F(MoveFinderTest, FindExchanges) {
  const Rack rack(tiles_->ToLetterString("AB").value());
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  const auto exchanges =
      move_finder_->FindExchanges(rack, MoveFinder::RecordAll);
  ExpectMoves(exchanges, {"EXCH A (score = 0)", "EXCH B (score = 0)",
                          "EXCH AB (score = 0)"});
}

TEST_F(MoveFinderTest, FindExchanges2) {
  const Rack rack(tiles_->ToLetterString("AAA").value());
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  const auto exchanges =
      move_finder_->FindExchanges(rack, MoveFinder::RecordAll);
  ExpectMoves(exchanges, {"EXCH A (score = 0)", "EXCH AA (score = 0)",
                          "EXCH AAA (score = 0)"});
}

TEST_F(MoveFinderTest, FindExchanges3) {
  const Rack rack(tiles_->ToLetterString("A?").value());
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  const auto exchanges =
      move_finder_->FindExchanges(rack, MoveFinder::RecordAll);
  ExpectMoves(exchanges, {"EXCH A (score = 0)", "EXCH ? (score = 0)",
                          "EXCH A? (score = 0)"});
}

TEST_F(MoveFinderTest, FindExchanges4) {
  const Rack rack(tiles_->ToLetterString("Q?").value());
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  const auto exchanges =
      move_finder_->FindExchanges(rack, MoveFinder::RecordBest);
  ExpectMoves(exchanges, {"EXCH Q (score = 0)"});
}
TEST_F(MoveFinderTest, FindWords) {
  Board board;
  const Rack rack(tiles_->ToLetterString("IIICFVV").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 7, 7);
  spot.SetWordMultiplier(2);
  const std::array<int, 7> tile_scores({4, 4, 4, 3, 1, 1, 1});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {"8H VIVIFIC (score = 94)"});
}

TEST_F(MoveFinderTest, FindWords2) {
  Board board;
  const Rack rack(tiles_->ToLetterString("BANANAS").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 6, 2);
  spot.SetWordMultiplier(2);
  const std::array<int, 7> tile_scores({3, 1, 1, 1, 1, 1, 1});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words,
              {"8G AA (score = 4)", "8G AB (score = 8)", "8G BA (score = 8)",
               "8G AN (score = 4)", "8G NA (score = 4)", "8G AS (score = 4)"});
}

TEST_F(MoveFinderTest, FindWords3) {
  Board board;
  const Rack rack(tiles_->ToLetterString("AEURVY?").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 5, 7);
  spot.SetWordMultiplier(2);
  const std::array<int, 7> tile_scores({4, 4, 1, 1, 1, 1, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {"8F qUAVERY (score = 82)"});
}

TEST_F(MoveFinderTest, FindWords4) {
  Board board;
  const Rack rack(tiles_->ToLetterString("SQIRT??").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 1, 7);
  spot.SetWordMultiplier(2);
  const std::array<int, 7> tile_scores({10, 1, 1, 1, 1, 0, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {"8B QInTaRS (score = 78)", "8B QueRIST (score = 78)",
                      "8B ReQuITS (score = 98)", "8B SQuIRTs (score = 78)",
                      "8B sQuIRTS (score = 78)"});
}

TEST_F(MoveFinderTest, ZeroPlayedThroughTiles) {
  Board board;
  const auto yag = Move::Parse("8G YAG", *tiles_);
  board.UnsafePlaceMove(yag.value());
  const auto before = tiles_->ToLetterString("SATYAGRAHA").value();
  EXPECT_TRUE(move_finder_->FitsWithPlayedThroughTiles(board, Move::Across, 7,
                                                       3, before));
  const auto after =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Across, 7, 3, before);
  EXPECT_EQ(after, tiles_->ToLetterString("SAT...RAHA").value());

  EXPECT_FALSE(move_finder_->FitsWithPlayedThroughTiles(board, Move::Across, 7,
                                                        4, before));
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
  EXPECT_TRUE(move_finder_->FitsWithPlayedThroughTiles(board, Move::Down, 3, 7,
                                                       before));
  const auto after =
      move_finder_->ZeroPlayedThroughTiles(board, Move::Down, 3, 7, before);
  EXPECT_EQ(after, tiles_->ToLetterString("SAT...RAHA").value());

  EXPECT_FALSE(move_finder_->FitsWithPlayedThroughTiles(board, Move::Down, 4, 7,
                                                        before));
}

TEST_F(MoveFinderTest, PlayThroughWithBlanks) {
  Board board;
  const auto con = Move::Parse("8G CON", *tiles_);
  board.UnsafePlaceMove(con.value());
  const Rack rack(tiles_->ToLetterString("URATES?").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 4, 7);
  spot.SetWordMultiplier(1);
  const std::array<int, 7> tile_scores({1, 1, 1, 1, 1, 1, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words,
              {"8E RA...TEUrS (score = 62)", "8E rA...TEURS (score = 62)",
               "8E RA...TEUSe (score = 62)", "8E RA...TeUSE (score = 62)"});
}

TEST_F(MoveFinderTest, CrossAt) {
  Board board;
  const auto ky = Move::Parse("8G KY", *tiles_);
  board.UnsafePlaceMove(ky.value());
  const auto cross_7G = move_finder_->CrossAt(board, Move::Across, 6, 6);
  ASSERT_TRUE(cross_7G.has_value());
  EXPECT_EQ(*cross_7G, LS(".K"));

  const auto oo = Move::Parse("9G oo", *tiles_);
  board.UnsafePlaceMove(oo.value());
  const auto cross_7H = move_finder_->CrossAt(board, Move::Across, 6, 7);
  ASSERT_TRUE(cross_7H.has_value());
  EXPECT_EQ(*cross_7H, LS(".Yo"));
}

TEST_F(MoveFinderTest, HookSum) {
  Board board;
  const auto ky = Move::Parse("8G KY", *tiles_);
  board.UnsafePlaceMove(ky.value());
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 6, 6, 3), 9);

  const auto oo = Move::Parse("9G oo", *tiles_);
  board.UnsafePlaceMove(oo.value());
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 6, 6, 3), 9);

  const auto beau = Move::Parse("10E BEAU", *tiles_);
  board.UnsafePlaceMove(beau.value());
  move_finder_->CacheCrossesAndScores(board);
  // 6 (Bx2) + 1 (E) + 6 (KoA) + 5 (YoU) = 19
  EXPECT_EQ(move_finder_->HookSum(board, Move::Across, 10, 0, 15), 18);
}

TEST_F(MoveFinderTest, CheckHooks) {
  Board board;
  const auto aa = Move::Parse("8G AA", *tiles_);
  board.UnsafePlaceMove(aa.value());

  const auto hm = Move::Parse("7G HM", *tiles_);
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_TRUE(move_finder_->CheckHooks(board, hm.value()));

  const auto ae = Move::Parse("7G AE", *tiles_);
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_TRUE(move_finder_->CheckHooks(board, ae.value()));

  const auto faan = Move::Parse("7F FAAN", *tiles_);
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_TRUE(move_finder_->CheckHooks(board, faan.value()));

  const auto qi = Move::Parse("7G QI", *tiles_);
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_FALSE(move_finder_->CheckHooks(board, qi.value()));

  const auto ax = Move::Parse("7G AX", *tiles_);
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_FALSE(move_finder_->CheckHooks(board, ax.value()));
}

TEST_F(MoveFinderTest, SevenTileOverlap) {
  Board board;
  const auto airtime = Move::Parse("8G AiRTIME", *tiles_);
  board.UnsafePlaceMove(airtime.value());

  const Rack rack(tiles_->ToLetterString("HEATER?").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 6, 6, 7);
  spot.SetWordMultiplier(1);
  const std::array<int, 7> tile_scores({4, 1, 1, 1, 1, 1, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {"7G tHEATER (score = 80)", "7G THEAtER (score = 82)"});
}

TEST_F(MoveFinderTest, NonHooks) {
  Board board;
  const auto vav = Move::Parse("8G VAV", *tiles_);
  board.UnsafePlaceMove(vav.value());

  const Rack rack(tiles_->ToLetterString("Z??").value());
  move_finder_->cross_map_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 6, 6, 3);
  const std::array<int, 7> tile_scores({10, 0, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {});
}

TEST_F(MoveFinderTest, FrontExtension) {
  Board board;
  const auto airtime = Move::Parse("8H DEMANDS", *tiles_);
  board.UnsafePlaceMove(airtime.value());

  const Rack rack(tiles_->ToLetterString("??UNTER").value());
  move_finder_->cross_map_.clear();
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  MoveFinder::Spot spot(Move::Across, 7, 0, 7);
  spot.SetWordMultiplier(3);
  const std::array<int, 7> tile_scores({1, 1, 1, 1, 1, 0, 0});
  move_finder_->ComputeSpotMaxEquity(rack, tile_scores, board, &spot);
  const auto words =
      move_finder_->FindWords(rack, board, spot, MoveFinder::RecordAll, 0);
  ExpectMoves(words, {"8A coUNTER....... (score = 101)"});
}

TEST_F(MoveFinderTest, EmptyBoardSpots) {
  Board board;
  const Rack rack(tiles_->ToLetterString("LULZ").value());
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
  const auto spots4 = move_finder_->FindSpots(rack, board);
  EXPECT_THAT(spots4,
              UnorderedElementsAre(MoveFinder::Spot(Move::Across, 7, 5, 3),
                                   MoveFinder::Spot(Move::Across, 7, 6, 3),
                                   MoveFinder::Spot(Move::Across, 7, 7, 3),
                                   MoveFinder::Spot(Move::Across, 7, 4, 4),
                                   MoveFinder::Spot(Move::Across, 7, 5, 4),
                                   MoveFinder::Spot(Move::Across, 7, 6, 4),
                                   MoveFinder::Spot(Move::Across, 7, 7, 4)));
}

TEST_F(MoveFinderTest, AcrossSpots) {
  Board board;
  // Doesn't actually matter what the rack is for what's being tested here, but
  // FindSpots will want to compute equity bounds with something.
  const Rack rack(tiles_->ToLetterString("WHATEVS").value());

  const auto an = Move::Parse("8H AN", *tiles_);
  board.UnsafePlaceMove(an.value());
  const auto ax = Move::Parse("9G AX", *tiles_);
  board.UnsafePlaceMove(ax.value());
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
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
  move_finder_->SetRackBits(rack);
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
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
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
  move_finder_->SetRackBits(rack);
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
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
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
  move_finder_->SetRackBits(rack);
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
  // Doesn't actually matter what the rack is for what's being tested here, but
  // FindSpots will want to compute equity bounds with something.
  const Rack rack(tiles_->ToLetterString("WHATEVS").value());
  const auto an = Move::Parse("8H AN", *tiles_);
  board.UnsafePlaceMove(an.value());
  move_finder_->CacheCrossesAndScores(board);
  const auto ax = Move::Parse("9G AX", *tiles_);
  board.UnsafePlaceMove(ax.value());
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
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
  move_finder_->subracks_.clear();
  move_finder_->CacheCrossesAndScores(board);
  move_finder_->CacheSubsets(rack);
  move_finder_->CacheRackPartitions(rack);
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
  move_finder_->cross_map_.clear();
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->WordScore(board, cwm.value(), 1), 10);

  board.UnsafePlaceMove(cwm.value());
  const auto euoi = Move::Parse("7H EUOI", *tiles_);
  // 2 + 4 + 1 + 1 = 8
  move_finder_->cross_map_.clear();
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->WordScore(board, euoi.value(), 1), 8);

  board.UnsafePlaceMove(euoi.value());
  const auto zol = Move::Parse("6J ZOL", *tiles_);
  // 60 + 2 + 1 = 63
  move_finder_->cross_map_.clear();
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->WordScore(board, zol.value(), 1), 63);

  board.UnsafePlaceMove(zol.value());
  const auto aka = Move::Parse("5J AKA", *tiles_);
  // 7*2 + 1 + 5*2 + 1 = 26
  move_finder_->cross_map_.clear();
  move_finder_->CacheCrossesAndScores(board);
  EXPECT_EQ(move_finder_->WordScore(board, aka.value(), 2), 26);
}

TEST_F(MoveFinderTest, FindMoves) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QUACKLE").value());
  move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordAll,
                          true);
  const std::vector<Move>& moves = move_finder_->Moves();
  // for (const auto& move : moves) {
  //   std::stringstream ss;
  //   move.Display(*tiles_, ss);
  //   LOG(INFO) << ss.str();
  // }
  // These are just the scoring plays and pass 0, no exchanges.
  int num_expected = 1 * 7 + 2 * 6 + 6 * 5 + 17 * 4 + 23 * 3 + 6 * 2;
  num_expected++;  // pass 0
  EXPECT_EQ(moves.size(), num_expected);
}

TEST_F(MoveFinderTest, FindMoves2) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QUACKLE").value());
  move_finder_->FindMoves(rack, board, *full_bag_, MoveFinder::RecordAll, true);
  const std::vector<Move>& moves = move_finder_->Moves();

  // for (const auto& move : moves) {
  //   std::stringstream ss;
  //   move.Display(*tiles_, ss);
  //   LOG(INFO) << ss.str();
  // }
  int num_expected = 1 * 7 + 2 * 6 + 6 * 5 + 17 * 4 + 23 * 3 + 6 * 2;
  num_expected += 128;  // exchanges and pass 0
  EXPECT_EQ(moves.size(), num_expected);
}

TEST_F(MoveFinderTest, FindBestExchange) {
  Board board;
  const Rack rack(tiles_->ToLetterString("RRRVVWW").value());
  move_finder_->FindMoves(rack, board, *full_bag_, MoveFinder::RecordAll, true);
  const std::vector<Move>& const_moves = move_finder_->Moves();

  int num_expected = 4 * 3 * 3;
  EXPECT_EQ(const_moves.size(), num_expected);

  std::vector<Move> moves = const_moves;
  std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
    return a.Equity() > b.Equity();
  });

  // Keep R is best
  EXPECT_EQ(moves[0].Letters(), LS("RRVVWW"));
  EXPECT_EQ(moves[0].Leave(), LS("R"));
  EXPECT_EQ(moves[0].Score(), 0.0);
  EXPECT_GE(moves[0].LeaveValue(), 0.0);
  EXPECT_GE(moves[0].Equity(), 0.0);

  // Keep VVWW is worst (except for Pass which is forced to be last)
  EXPECT_EQ(moves[34].Letters(), LS("RRR"));
  EXPECT_EQ(moves[34].Leave(), LS("VVWW"));
  EXPECT_EQ(moves[34].Score(), 0.0);
  EXPECT_LE(moves[34].LeaveValue(), 0.0);
  EXPECT_LE(moves[34].Equity(), 0.0);

  // Pass is last
  EXPECT_EQ(moves[35].Letters(), LS(""));
  EXPECT_EQ(moves[35].Leave(), LS("RRRVVWW"));
  EXPECT_EQ(moves[35].Score(), 0.0);
  EXPECT_FLOAT_EQ(moves[35].LeaveValue(), -1000.0);
  EXPECT_FLOAT_EQ(moves[35].Equity(), -1000.0);
}

TEST_F(MoveFinderTest, FindBestMove) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QCLEANS").value());
  move_finder_->FindMoves(rack, board, *full_bag_, MoveFinder::RecordAll, true);
  const std::vector<Move>& const_moves = move_finder_->Moves();
  std::vector<Move> moves = const_moves;
  std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
    return a.Equity() > b.Equity();
  });

  const Move* best_nonexchange = nullptr;
  for (const auto& move : moves) {
    if (move.GetAction() != Move::Exchange) {
      best_nonexchange = &move;
      break;
    }
  }
  ExpectMove(moves[0], "EXCH Q (score = 0)");
  ExpectMove(*best_nonexchange, "8D CLEAN (score = 20)");
}

TEST_F(MoveFinderTest, RecordBest1) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QCLEANS").value());
  move_finder_->FindMoves(rack, board, *full_bag_, MoveFinder::RecordBest,
                          true);
  const std::vector<Move>& moves = move_finder_->Moves();
  ExpectMoves(moves, {"EXCH Q (score = 0)"});
}

TEST_F(MoveFinderTest, RecordBest2) {
  Board board;
  const Rack rack(tiles_->ToLetterString("QANASTA").value());
  move_finder_->FindMoves(rack, board, *full_bag_, MoveFinder::RecordBest,
                          true);
  const std::vector<Move>& moves = move_finder_->Moves();
  ExpectMoves(moves, {"8D QANAT (score = 48)"});
}

TEST_F(MoveFinderTest, RepeatedlyPlay1) {
  Board board;
  int moves_played = 0;
  for (int i = 0;; ++i) {
    bool recompute_crosses = (i == 0);
    const Rack rack(tiles_->ToLetterString("RATES??").value());
    move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordBest,
                            recompute_crosses);
    const std::vector<Move>& moves = move_finder_->Moves();
    ASSERT_EQ(moves.size(), 1);
    std::stringstream ss;
    moves[0].Display(*tiles_, ss);
    LOG(INFO) << "moves[0]: " << ss.str();
    if (moves[0].GetAction() != Move::Place) {
      break;
    }
    board.UnsafePlaceMove(moves[0]);
    move_finder_->CacheCrossesAndScores(board, moves[0]);
    moves_played++;
  }
  LOG(INFO) << "Played " << moves_played << " moves";
  std::stringstream ss;
  board_layout_->DisplayBoard(board, *tiles_, ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_EQ(moves_played, 45);
}

TEST_F(MoveFinderTest, RepeatedlyPlay2) {
  Board board;
  int moves_played = 0;
  for (int i = 0;; ++i) {
    bool recompute_crosses = (i == 0);
    const Rack rack(tiles_->ToLetterString("PLUMBUM").value());
    move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordBest,
                            recompute_crosses);
    const std::vector<Move>& moves = move_finder_->Moves();
    ASSERT_EQ(moves.size(), 1);
    std::stringstream ss;
    moves[0].Display(*tiles_, ss);
    LOG(INFO) << "moves[0]: " << ss.str();
    if (moves[0].GetAction() != Move::Place) {
      break;
    }
    board.UnsafePlaceMove(moves[0]);
    move_finder_->CacheCrossesAndScores(board, moves[0]);
    moves_played++;
  }
  LOG(INFO) << "Played " << moves_played << " moves";
  std::stringstream ss;
  board_layout_->DisplayBoard(board, *tiles_, ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_EQ(moves_played, 46);
}

TEST_F(MoveFinderTest, RepeatedlyPlay3) {
  Board board;
  int moves_played = 0;
  for (int i = 0;; ++i) {
    bool recompute_crosses = (i == 0);
    const Rack rack(tiles_->ToLetterString("AAAAAAA").value());
    move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordBest,
                            recompute_crosses);
    const std::vector<Move>& moves = move_finder_->Moves();
    ASSERT_EQ(moves.size(), 1);
    std::stringstream ss;
    moves[0].Display(*tiles_, ss);
    LOG(INFO) << "moves[0]: " << ss.str();
    if (moves[0].GetAction() != Move::Place) {
      break;
    }
    board.UnsafePlaceMove(moves[0]);
    move_finder_->CacheCrossesAndScores(board, moves[0]);
    moves_played++;
  }
  LOG(INFO) << "Played " << moves_played << " moves";
  std::stringstream ss;
  board_layout_->DisplayBoard(board, *tiles_, ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_EQ(moves_played, 2);
}

TEST_F(MoveFinderTest, PlayThroughBlank) {
  Board board;
  const auto jetbead = Move::Parse("8D JeTBeAD", *tiles_);
  board.UnsafePlaceMove(jetbead.value());
  const Rack rack(tiles_->ToLetterString("OXAZPAM").value());
  move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordBest,
                          true);
  const std::vector<Move>& moves = move_finder_->Moves();
  ExpectMoves(moves, {"E4 OXAZ.PAM (score = 158)"});
}

TEST_F(MoveFinderTest, CacheCrossesAndScores) {
  Board board;
  move_finder_->CacheCrossesAndScores(board);
  for (int dir = 0; dir < 2; dir++) {
    for (int row = 0; row < 15; row++) {
      for (int col = 0; col < 15; col++) {
        EXPECT_EQ(move_finder_->hook_table_[dir][row][col], kNotTouching);
        EXPECT_EQ(move_finder_->score_table_[dir][row][col], 0);
      }
    }
  }

  const auto qi = Move::Parse("8H QI", *tiles_);
  board.UnsafePlaceMove(qi.value());
  move_finder_->CacheCrossesAndScores(board);
  const int kAcross = 0;
  const int kDown = 1;
  for (int dir : {kAcross, kDown}) {
    for (int row = 0; row < 15; row++) {
      for (int col = 0; col < 15; col++) {
        auto sq = std::make_tuple(dir, row, col);
        // LOG(INFO) << "sq: " << dir << " " << row << " " << col;
        if (sq == std::make_tuple(kAcross, 6, 7)) {
          // _Q = {}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], 0);
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("Q"));
        } else if (sq == std::make_tuple(kAcross, 6, 8)) {
          // _I = {ABDGHKLMPOQSTX}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col],
                    H("ABDGHKLMPOQSTX"));
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("I"));
        } else if (sq == std::make_tuple(kAcross, 8, 7)) {
          // Q_ = {I}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], H("I"));
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("Q"));
        } else if (sq == std::make_tuple(kAcross, 8, 8)) {
          // I_ = {DFNOST}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], H("DFNOST"));
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("I"));
        } else if (sq == std::make_tuple(kDown, 7, 6)) {
          // _QI = {}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], 0);
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("QI"));
        } else if (sq == std::make_tuple(kDown, 7, 9)) {
          // QI_ = {NS}
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], H("NS"));
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], Score("QI"));
        } else {
          EXPECT_EQ(move_finder_->hook_table_[dir][row][col], kNotTouching);
          EXPECT_EQ(move_finder_->score_table_[dir][row][col], 0);
        }
      }
    }
  }
  // EXPECT_TRUE(false);
}

TEST_F(MoveFinderTest, FindMissingMove) {
  Board board;
  const auto abaters = Move::Parse("8B abATERS", *tiles_);
  board.UnsafePlaceMove(abaters.value());
  const auto rabattes = Move::Parse("E4 RabA.TES", *tiles_);
  board.UnsafePlaceMove(rabattes.value());
  const auto gastreas = Move::Parse("H1 gaSTREA.", *tiles_);
  board.UnsafePlaceMove(gastreas.value());
  const auto erostrate = Move::Parse("4D e.oS.RATE", *tiles_);
  board.UnsafePlaceMove(erostrate.value());
  const auto tabarets = Move::Parse("K4 .abARETS", *tiles_);
  board.UnsafePlaceMove(tabarets.value());
  const auto aerates = Move::Parse("L7 aERATeS", *tiles_);
  board.UnsafePlaceMove(aerates.value());
  const Rack rack(tiles_->ToLetterString("RATES??").value());
  move_finder_->FindMoves(rack, board, *empty_bag_, MoveFinder::RecordBest,
                          true);
  const std::vector<Move>& moves = move_finder_->Moves();
  EXPECT_EQ(moves[0].Score(), 72);  // M5 mASTERy or wASTERy
}
