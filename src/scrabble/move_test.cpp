#include "src/scrabble/move.h"

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/tiles.h"

class MoveTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }
  std::unique_ptr<Tiles> tiles_;
};

TEST_F(MoveTest, Pass) {
  Move move;
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0");
}

TEST_F(MoveTest, Exchange) {
  Move move(tiles_->ToLetterString("UUVW").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW");
}

TEST_F(MoveTest, PlaceAcross) {
  Move move(Move::Across, 7, 3, tiles_->ToLetterString("QuACKLe").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "8D QuACKLe");
}

TEST_F(MoveTest, PlaceAcrossRow10) {
  Move move(Move::Across, 9, 3, tiles_->ToLetterString("QuACKLe").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "10D QuACKLe");
}

TEST_F(MoveTest, PlaceDown) {
  Move move(Move::Down, 9, 9, tiles_->ToLetterString("YUNXES").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "J10 YUNXES");
}

TEST_F(MoveTest, ParsePASS) {
  const auto move_or = Move::Parse("PASS", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0");
}

TEST_F(MoveTest, ParsePASS_0) {
  const auto move_or = Move::Parse("pAsS 0", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0");
}

TEST_F(MoveTest, ParseDash) {
  const auto move_or = Move::Parse("-", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0");
}

TEST_F(MoveTest, ParseEXCH_UUVW) {
  const auto move_or = Move::Parse("EXCH UUVW", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW");
}

TEST_F(MoveTest, ParseDashUUVW) {
  const auto move_or = Move::Parse("-UUVW", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW");
}

TEST_F(MoveTest, ParseExch_uuvw) {
  const auto move_or = Move::Parse("exch uuvw", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW");
}

TEST_F(MoveTest, ParseDashuUVw) {
  const auto move_or = Move::Parse("-uUVw", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW");
}

TEST_F(MoveTest, ParsePlace10J) {
  const auto move_or = Move::Parse("10J FoO", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "10J FoO");
}

TEST_F(MoveTest, ParsePlaceA1) {
  const auto move_or = Move::Parse("A1 FoO", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "A1 FoO");
}

TEST_F(MoveTest, ParsePlaceJ10) {
  const auto move_or = Move::Parse("j10 FoO", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "J10 FoO");
}

TEST_F(MoveTest, ParsePlace1A) {
  const auto move_or = Move::Parse("1a FoO", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "1A FoO");
}