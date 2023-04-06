#include "src/scrabble/game_position.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

std::unique_ptr<Tiles> tiles_;
std::unique_ptr<BoardLayout> layout_;
class GamePositionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    layout_ = absl::make_unique<BoardLayout>(
        "src/scrabble/testdata/scrabble_board.textproto");
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }
};

TEST_F(GamePositionTest, Display) {
  Board board;
  Rack rack(tiles_->ToLetterString("OLAUGHS").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             nullptr, absl::Minutes(25), *tiles_);
  std::stringstream ss1;
  pos->Display(ss1);
  LOG(INFO) << std::endl << ss1.str();
  EXPECT_EQ(ss1.str(), R"(  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　　　－　　　＇　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
Player 1 holds OLAUGHS on 0 to opp's 0 [25:00]
Unseen: AAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGHIIIIIIIIIJKLLLMMNNNNNNOOOOOOOPPQRRRRRRSSSTTTTTTUUUVVWWXYYZ??
)");

  // The move's score isn't calculated in its creation, so it's not displayed.
  const auto goulash = Move::Parse("8F GOULASH", *tiles_);
  pos->CommitMove(goulash.value(), absl::Seconds(123));
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << std::endl << ss2.str();
  EXPECT_EQ(ss2.str(), R"(  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　　　－　　　＇　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
Player 1 holding OLAUGHS on 0 to opp's 0 plays 8F GOULASH [25:00 → 22:57]
Unseen (before drawing): AAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGHIIIIIIIIIJKLLLMMNNNNNNOOOOOOOPPQRRRRRRSSSTTTTTTUUUVVWWXYYZ??
)");
}