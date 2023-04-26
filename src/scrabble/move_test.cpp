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
  LetterString empty;
  Move move(empty);
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0 (score = 0)");
}

TEST_F(MoveTest, Exchange) {
  Move move(tiles_->ToLetterString("UUVW").value());
  std::stringstream ss;
  move.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW (score = 0)");
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
  EXPECT_EQ(ss.str(), "PASS 0 (score = 0)");
}

TEST_F(MoveTest, ParsePASS_0) {
  const auto move_or = Move::Parse("pAsS 0", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0 (score = 0)");
}

TEST_F(MoveTest, ParseDash) {
  const auto move_or = Move::Parse("-", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "PASS 0 (score = 0)");
}

TEST_F(MoveTest, ParseEXCH_UUVW) {
  const auto move_or = Move::Parse("EXCH UUVW", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW (score = 0)");
}

TEST_F(MoveTest, ParseDashUUVW) {
  const auto move_or = Move::Parse("-UUVW", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW (score = 0)");
}

TEST_F(MoveTest, ParseExch_uuvw) {
  const auto move_or = Move::Parse("exch uuvw", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW (score = 0)");
}

TEST_F(MoveTest, ParseDashuUVw) {
  const auto move_or = Move::Parse("-uUVw", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "EXCH UUVW (score = 0)");
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

TEST_F(MoveTest, ParsePlaythrough) {
  const auto move_or = Move::Parse("o8 ..IPLAPS", *tiles_);
  ASSERT_TRUE(move_or.ok());
  std::stringstream ss;
  move_or->Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "O8 ..IPLAPS");
}

TEST_F(MoveTest, Deadwood) {
  Move bonus(Move::OppDeadwoodBonus, tiles_->ToLetterString("BNOSU").value(),
             14);
  std::stringstream ss1;
  bonus.Display(*tiles_, ss1);
  EXPECT_EQ(ss1.str(), "{BNOSU} (score = 14)");

  Move penalty(Move::OwnDeadwoodPenalty,
               tiles_->ToLetterString("AELNPTY").value(), -12);
  std::stringstream ss2;
  penalty.Display(*tiles_, ss2);
  EXPECT_EQ(ss2.str(), "[AELNPTY] (score = -12)");
}

TEST_F(MoveTest, WithScore) {
  Move quackle(Move::Across, 7, 3, tiles_->ToLetterString("QuACKLe").value(),
               110);
  std::stringstream ss1;
  quackle.Display(*tiles_, ss1);
  EXPECT_EQ(ss1.str(), "8D QuACKLe (score = 110)");

  Move cow(Move::Across, 7, 6, tiles_->ToLetterString("COW").value());
  cow.SetScore(16);
  std::stringstream ss2;
  cow.Display(*tiles_, ss2);
  EXPECT_EQ(ss2.str(), "8G COW (score = 16)");
}

TEST_F(MoveTest, Assignable) {
  const Move a(Move::Across, 7, 6, tiles_->ToLetterString("COW").value());
  const Move b = a;
  std::stringstream ss;
  b.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "8G COW");
}

TEST_F(MoveTest, Copyable) {
  const Move a(Move::Across, 7, 6, tiles_->ToLetterString("COW").value());
  const Move b(a);
  std::stringstream ss;
  b.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "8G COW");
}

TEST_F(MoveTest, Moveable) {
  Move a(Move::Across, 7, 6, tiles_->ToLetterString("COW").value());
  Move b(Move::Across, 7, 6, tiles_->ToLetterString("ZZZ").value());
  b = std::move(a);
  std::stringstream ss;
  b.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "8G COW");
}

TEST_F(MoveTest, Swappable) {
  std::vector<Move> moves(
      {Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 4),
       Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 2)});
  std::swap(moves[0], moves[1]);
  EXPECT_EQ(moves[0].Score(), 2);
  EXPECT_EQ(moves[1].Score(), 4);
}

TEST_F(MoveTest, Sortable) {
  std::vector<Move> moves(
      {Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 4),
       Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 2),
       Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 3),
       Move(Move::Across, 0, 0, tiles_->ToLetterString("XX").value(), 9)});
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.Score() > b.Score(); });
  EXPECT_EQ(moves[0].Score(), 9);
  EXPECT_EQ(moves[1].Score(), 4);
  EXPECT_EQ(moves[2].Score(), 3);
  EXPECT_EQ(moves[3].Score(), 2);
}