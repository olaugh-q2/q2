#include "src/scrabble/bag.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class BagTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(BagTest, Construction) {
  const Bag bag(*tiles_);
  EXPECT_EQ(bag.Size(), 100);
  std::stringstream ss;
  bag.Display(ss);
  EXPECT_EQ(ss.str(),
            "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
            "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");

  EXPECT_TRUE(bag.CanExchange());
}

TEST_F(BagTest, Construction2) {
  const Bag bag(*tiles_, {});
  EXPECT_EQ(bag.Size(), 0);
  std::stringstream ss;
  bag.Display(ss);
  EXPECT_EQ(ss.str(), "");
  EXPECT_FALSE(bag.CanExchange());
}

TEST_F(BagTest, Construction3) {
  const Bag bag(*tiles_, {L('A'), L('B'), L('?')});
  EXPECT_EQ(bag.Size(), 3);
  std::stringstream ss;
  bag.Display(ss);
  EXPECT_EQ(ss.str(), "AB?");
  EXPECT_FALSE(bag.CanExchange());
}