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

TEST_F(MoveTest, Place) {
  Move move(Move::Across, 7, 3, tiles_->ToLetterString("QuACKLe").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "8D QuACKLe");
}