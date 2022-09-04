#include "src/scrabble/board_layout.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ElementsAreArray;

class BoardLayoutTest : public testing::Test {
 protected:
  void SetUp() override {
    layout_ = absl::make_unique<BoardLayout>(
        "src/scrabble/testdata/scrabble_board.textproto");
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }
  std::unique_ptr<BoardLayout> layout_;
  std::unique_ptr<Tiles> tiles_;
};

TEST_F(BoardLayoutTest, Construction) {
  const auto& lm = layout_->letter_multipliers_;
  EXPECT_THAT(lm[0],
              ElementsAreArray({1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1}));
  EXPECT_THAT(lm[1],
              ElementsAreArray({1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1}));
  EXPECT_THAT(lm[2],
              ElementsAreArray({1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1}));
  EXPECT_THAT(lm[3],
              ElementsAreArray({2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2}));
  EXPECT_THAT(lm[4],
              ElementsAreArray({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}));
  EXPECT_THAT(lm[5],
              ElementsAreArray({1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1}));
}

TEST_F(BoardLayoutTest, DisplayEmptyRow1) {
  Board board;
  std::stringstream ss;
  layout_->DisplayRow(board, 0, *tiles_, ss);
  EXPECT_EQ(ss.str(), " 1＝　　＇　　　＝　　　＇　　＝");
}

TEST_F(BoardLayoutTest, DisplayEmptyRow10) {
  Board board;
  std::stringstream ss;
  layout_->DisplayRow(board, 9, *tiles_, ss);
  EXPECT_EQ(ss.str(), "10　＂　　　＂　　　＂　　　＂　");
}

TEST_F(BoardLayoutTest, DisplayHeader) {
  Board board;
  std::stringstream ss;
  layout_->DisplayHeader(ss);
  EXPECT_EQ(ss.str(), "  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ");
}