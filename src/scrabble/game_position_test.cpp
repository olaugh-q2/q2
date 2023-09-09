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
  auto pos = absl::make_unique<GamePosition>(
      *layout_, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles_);
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

TEST_F(GamePositionTest, DisplayScorelessTurns) {
  Board board;
  Rack rack(tiles_->ToLetterString("OLAUGHS").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             0, 0, absl::Minutes(25), 5, *tiles_);
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
[Consecutive scoreless turns preceding this position: 5]
Player 1 holds OLAUGHS on 0 to opp's 0 [25:00]
Unseen: AAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGHIIIIIIIIIJKLLLMMNNNNNNOOOOOOOPPQRRRRRRSSSTTTTTTUUUVVWWXYYZ??
)");

  const auto pass = Move::Parse("PASS 0", *tiles_);
  pos->CommitMove(pass.value(), absl::Seconds(123));
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
[Consecutive scoreless turns preceding this position: 5]
This move makes it 6 consecutive scoreless turns.
Player 1 holding OLAUGHS on 0 to opp's 0 plays PASS 0 (score = 0) [25:00 → 22:57]
Unseen (before drawing): AAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGHIIIIIIIIIJKLLLMMNNNNNNOOOOOOOPPQRRRRRRSSSTTTTTTUUUVVWWXYYZ??
)");
}

TEST_F(GamePositionTest, ScorelessTurn1) {
  Board board;
  Rack rack(tiles_->ToLetterString("JKQVVXZ").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             0, 0, absl::Minutes(25), 0, *tiles_);
  const auto exchange = Move::Parse("EXCH JKQVVXZ", *tiles_);
  pos->CommitMove(exchange.value(), absl::Microseconds(200));
  std::stringstream ss;
  pos->Display(ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_TRUE(pos->IsScorelessTurn());
}

TEST_F(GamePositionTest, ScorelessTurn2) {
  Board board;
  Rack rack(tiles_->ToLetterString("JKQVVXZ").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             0, 0, absl::Minutes(25), 0, *tiles_);
  const auto pass = Move::Parse("PASS 0", *tiles_);
  pos->CommitMove(pass.value(), absl::Microseconds(200));
  std::stringstream ss;
  pos->Display(ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_TRUE(pos->IsScorelessTurn());
}

TEST_F(GamePositionTest, NotScorelessTurn) {
  Board board;
  Rack rack(tiles_->ToLetterString("OLAUGHS").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             0, 0, absl::Minutes(25), 0, *tiles_);
  const auto goulash = Move::Parse("8F GOULASH", *tiles_);
  pos->CommitMove(goulash.value(), absl::Microseconds(200));
  std::stringstream ss;
  pos->Display(ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_FALSE(pos->IsScorelessTurn());
}