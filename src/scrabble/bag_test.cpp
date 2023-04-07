#include "src/scrabble/bag.h"

#include <limits>

#include "absl/memory/memory.h"
#include "absl/random/mocking_bit_gen.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Return;

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

TEST_F(BagTest, UnseenToPlayer) {
  Board board;
  const auto plumbum = Move::Parse("8D PLUMBUM", *tiles_);
  board.UnsafePlaceMove(plumbum.value());
  Rack rack(tiles_->ToLetterString("QUACKLE").value());
  auto unseen = Bag::UnseenToPlayer(*tiles_, board, rack);
  ASSERT_NE(unseen, nullptr);
  EXPECT_EQ(unseen->Size(), 100 - 14);
}

TEST_F(BagTest, Shuffle) {
  Bag bag(*tiles_);
  // I don't know how to make this deterministic :(
  absl::MockingBitGen gen;
  bag.Shuffle(gen);
  EXPECT_EQ(bag.Size(), 100);
  std::stringstream ss;
  bag.Display(ss);
  EXPECT_NE(ss.str(),
            "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
            "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");
}

TEST_F(BagTest, CompleteRack1) {
  Bag bag(*tiles_);
  Rack rack(tiles_->ToLetterString("").value());
  bag.CompleteRack(&rack);
  EXPECT_EQ(bag.Size(), 100 - 7);
  EXPECT_EQ(rack.NumTiles(), 7);
  std::stringstream ss;
  rack.Display(*tiles_, ss);
  // Popped off the end of our unshuffled bag.
  EXPECT_EQ(ss.str(), "??ZYYXW");
}

TEST_F(BagTest, CompleteRack2) {
  Bag bag(*tiles_);
  // Nonsense and not removed from bag but doesn't matter.
  Rack rack(tiles_->ToLetterString("QQQQ").value());
  bag.CompleteRack(&rack);
  EXPECT_EQ(bag.Size(), 100 - 3);
  EXPECT_EQ(rack.NumTiles(), 7);
  std::stringstream ss;
  rack.Display(*tiles_, ss);
  // Popped off the end of our unshuffled bag.
  EXPECT_EQ(ss.str(), "QQQQ??Z");
}

TEST_F(BagTest, SetLetters) {
  Bag bag(*tiles_);
  bag.SetLetters({L('A'), L('B'), L('C')});
  EXPECT_EQ(bag.Size(), 3);
  std::stringstream ss;
  bag.Display(ss);
  EXPECT_EQ(ss.str(), "ABC");
}