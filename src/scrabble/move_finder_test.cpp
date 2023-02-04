#include "src/scrabble/move_finder.h"

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;

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
  Rack rack(tiles_->ToLetterString("AB").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH B", "EXCH AB"});
}

TEST_F(MoveFinderTest, FindExchanges2) {
  Rack rack(tiles_->ToLetterString("AAA").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH AA", "EXCH AAA"});
}

TEST_F(MoveFinderTest, FindExchanges3) {
  Rack rack(tiles_->ToLetterString("A?").value());
  const auto exchanges = move_finder_->FindExchanges(rack);
  ExpectMoves(exchanges, {"PASS 0", "EXCH A", "EXCH ?", "EXCH A?"});
}

TEST_F(MoveFinderTest, FindWords) {
  Board board;
  Rack rack(tiles_->ToLetterString("IIICFVV").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 7, 7);
  ExpectMoves(words, {"8H VIVIFIC"});
}

TEST_F(MoveFinderTest, FindWords2) {
  Board board;
  Rack rack(tiles_->ToLetterString("BANANAS").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 6, 2);
  ExpectMoves(words, {"8G AA", "8G AB", "8G BA", "8G AN", "8G NA", "8G AS"});
}

TEST_F(MoveFinderTest, FindWords3) {
  Board board;
  Rack rack(tiles_->ToLetterString("AEURVY?").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 5, 7);
  ExpectMoves(words, {"8F qUAVERY"});
}

TEST_F(MoveFinderTest, FindWords4) {
  Board board;
  Rack rack(tiles_->ToLetterString("SQIRT??").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 1, 7);
  ExpectMoves(words, {"8B QInTaRS", "8B QueRIST", "8B ReQuITS", "8B SQuIRTs",
                      "8B sQuIRTS"});
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
  Rack rack(tiles_->ToLetterString("URATES?").value());
  const auto words =
      move_finder_->FindWords(rack, board, Move::Across, 7, 4, 7);
  ExpectMoves(words, {"8E RA...TEUrS", "8E rA...TEURS", "8E RA...TEUSe",
                      "8E RA...TeUSE"});
}