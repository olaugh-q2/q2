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
  Rack rack(tiles_->ToLetterString("GOULASH").value());
  auto pos = absl::make_unique<GamePosition>(*layout_, board, 1, 2, rack, 0, 0,
                                             nullptr, *tiles_);
  std::stringstream ss;
  pos->Display(ss);
  LOG(INFO) << std::endl << ss.str();
  EXPECT_EQ(ss.str(), "  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ");
}