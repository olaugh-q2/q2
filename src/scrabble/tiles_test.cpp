#include "tiles.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class TilesTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJ"
        "KLLLLMMMMMMMMNNNNNNOOOOOOOOPPQRRRRRRRRSSSS"
        "TTTTTTUUUUVVWWXYYZ??");
  }
  std::unique_ptr<Tiles> tiles_;
};

TEST_F(TilesTest, CharToNumber) {
  EXPECT_EQ(tiles_->CharToNumber('A'), 1);
  EXPECT_EQ(tiles_->CharToNumber('B'), 2);
  EXPECT_EQ(tiles_->CharToNumber('Z'), 26);
  EXPECT_EQ(tiles_->CharToNumber('?'), 27);

  EXPECT_EQ(tiles_->CharToNumber('a'), absl::nullopt);
  EXPECT_EQ(tiles_->CharToNumber('_'), absl::nullopt);
  EXPECT_EQ(tiles_->CharToNumber(' '), absl::nullopt);
  EXPECT_EQ(tiles_->CharToNumber('\0'), absl::nullopt);
}

TEST_F(TilesTest, NumberToChar) {
  EXPECT_EQ(tiles_->NumberToChar(1), 'A');
  EXPECT_EQ(tiles_->NumberToChar(2), 'B');
  EXPECT_EQ(tiles_->NumberToChar(26), 'Z');
  EXPECT_EQ(tiles_->NumberToChar(27), '?');

  EXPECT_EQ(tiles_->NumberToChar(0), absl::nullopt);
  EXPECT_EQ(tiles_->NumberToChar(28), absl::nullopt);
}

TEST_F(TilesTest, Distribution) {
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('A').value()), 9);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('B').value()), 2);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('Z').value()), 1);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('?').value()), 2);
}

TEST_F(TilesTest, PrimeIndices) {
  const auto prime_indices = tiles_->PrimeIndices();
  EXPECT_EQ(prime_indices[0], 0);  // empty

  // 12 E's is the most, it gets the lowest prime
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('E').value()], 1);

  // 9 A's and 9 I's, but A is first alphabetically, which breaks the tie.
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('A').value()], 2);
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('I').value()], 3);
}
