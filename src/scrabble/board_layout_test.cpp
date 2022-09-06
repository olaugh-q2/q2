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

TEST_F(BoardLayoutTest, UnsafePlaceMove) {
  Board board;
  std::array<std::string, 15> before_row_strings;
  for (int i = 0; i < 15; ++i) {
    std::stringstream ss;
    layout_->DisplayRow(board, i, *tiles_, ss);
    before_row_strings[i] = ss.str();
  }
  const auto& citizen = Move::Parse("8h CITIZEN", *tiles_);
  board.UnsafePlaceMove(citizen.value());
  std::array<std::string, 15> citizen_row_strings;
  for (int i = 0; i < 15; ++i) {
    std::stringstream ss;
    layout_->DisplayRow(board, i, *tiles_, ss);
    citizen_row_strings[i] = ss.str();
  }
  for (int i = 0; i < 15; ++i) {
    if (i == 7) {
      EXPECT_EQ(citizen_row_strings[i], " 8＝　　＇　　　ＣＩＴＩＺＥＮ＝");
    } else {
      EXPECT_EQ(citizen_row_strings[i], before_row_strings[i]);
    }
  }
  const auto& sh = Move::Parse("o8 SH", *tiles_);
  board.UnsafePlaceMove(sh.value());
  std::array<std::string, 15> sh_row_strings;
  for (int i = 0; i < 15; ++i) {
    std::stringstream ss;
    layout_->DisplayRow(board, i, *tiles_, ss);
    sh_row_strings[i] = ss.str();
  }
  for (int i = 0; i < 15; ++i) {
    if (i == 7) {
      EXPECT_EQ(sh_row_strings[i], " 8＝　　＇　　　ＣＩＴＩＺＥＮＳ");
    } else if (i == 8) {
      EXPECT_EQ(sh_row_strings[i], " 9　　＇　　　＇　＇　　　＇　Ｈ");
    } else {
      EXPECT_EQ(sh_row_strings[i], before_row_strings[i]);
    }
  }
  const auto& shiplaps = Move::Parse("o8 ..IPLAPS", *tiles_);
  board.UnsafePlaceMove(shiplaps.value());
  std::array<std::string, 15> shiplaps_row_strings;
  for (int i = 0; i < 15; ++i) {
    std::stringstream ss;
    layout_->DisplayRow(board, i, *tiles_, ss);
    shiplaps_row_strings[i] = ss.str();
  }
  for (int i = 0; i < 15; ++i) {
    if (i == 7) {
      EXPECT_EQ(shiplaps_row_strings[i], " 8＝　　＇　　　ＣＩＴＩＺＥＮＳ");
    } else if (i == 8) {
      EXPECT_EQ(shiplaps_row_strings[i], " 9　　＇　　　＇　＇　　　＇　Ｈ");
    } else if (i == 9) {
      EXPECT_EQ(shiplaps_row_strings[i], "10　＂　　　＂　　　＂　　　＂Ｉ");
    } else if (i == 10) {
      EXPECT_EQ(shiplaps_row_strings[i], "11　　　　－　　　　　－　　　Ｐ");
    } else if (i == 11) {
      EXPECT_EQ(shiplaps_row_strings[i], "12＇　　－　　　＇　　　－　　Ｌ");
    } else if (i == 12) {
      EXPECT_EQ(shiplaps_row_strings[i], "13　　－　　　＇　＇　　　－　Ａ");
    } else if (i == 13) {
      EXPECT_EQ(shiplaps_row_strings[i], "14　－　　　＂　　　＂　　　－Ｐ");
    } else if (i == 14) {
      EXPECT_EQ(shiplaps_row_strings[i], "15＝　　＇　　　＝　　　＇　　Ｓ");
    }
  }
}