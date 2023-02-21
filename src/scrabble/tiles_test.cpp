#include "src/scrabble/tiles.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class TilesTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }
  
  std::unique_ptr<Tiles> tiles_;
};

TEST_F(TilesTest, CharToNumber) {
  EXPECT_EQ(tiles_->CharToNumber('A'), 1);
  EXPECT_EQ(tiles_->CharToNumber('B'), 2);
  EXPECT_EQ(tiles_->CharToNumber('Z'), 26);
  EXPECT_EQ(tiles_->CharToNumber('?'), 27);

  EXPECT_EQ(tiles_->CharToNumber('a'), 27 + 1);
  EXPECT_EQ(tiles_->CharToNumber('b'), 27 + 2);
  EXPECT_EQ(tiles_->CharToNumber('z'), 27 + 26);

  EXPECT_EQ(tiles_->CharToNumber('_'), absl::nullopt);
  EXPECT_EQ(tiles_->CharToNumber(' '), absl::nullopt);
  EXPECT_EQ(tiles_->CharToNumber('\0'), absl::nullopt);
}

TEST_F(TilesTest, NumberToChar) {
  EXPECT_EQ(tiles_->NumberToChar(1), 'A');
  EXPECT_EQ(tiles_->NumberToChar(2), 'B');
  EXPECT_EQ(tiles_->NumberToChar(26), 'Z');
  EXPECT_EQ(tiles_->NumberToChar(27), '?');
  EXPECT_EQ(tiles_->NumberToChar(27 + 1), 'a');
  EXPECT_EQ(tiles_->NumberToChar(27 + 26), 'z');

  EXPECT_EQ(tiles_->NumberToChar(0), '.');

  EXPECT_EQ(tiles_->NumberToChar(27 + 27), absl::nullopt);
  EXPECT_EQ(tiles_->NumberToChar(127), absl::nullopt);
}

TEST_F(TilesTest, ToLetterString) {
  char quackle[8] = "QUACKLE";
  char numbers[8] = "QUACKLE";
  for (int i = 0; i < 7; i++) {
    numbers[i] = quackle[i] - 'A' + 1;
  }
  auto letter_string = tiles_->ToLetterString(quackle);
  ASSERT_TRUE(letter_string.has_value());
  const LetterString expected(numbers, 7);
  EXPECT_EQ(letter_string.value(), expected);
}

TEST_F(TilesTest, ToLetterStringWithBlank) {
  char quackle[8] = "QUACKLE";
  char numbers[8] = "QUACKLE";
  for (int i = 0; i < 7; i++) {
    numbers[i] = quackle[i] - 'A' + 1;
  }
  quackle[0] = 'q';
  numbers[0] += 27;
  auto letter_string = tiles_->ToLetterString(quackle);
  ASSERT_TRUE(letter_string.has_value());
  const LetterString expected(numbers, 7);
  EXPECT_EQ(letter_string.value(), expected);
}

TEST_F(TilesTest, ToLetterStringWithPlaythrough) {
  char quackle[8] = ".UACKLE";
  char numbers[8] = "QUACKLE";
  for (int i = 0; i < 7; i++) {
    numbers[i] = quackle[i] - 'A' + 1;
  }
  numbers[0] = 0;
  auto letter_string = tiles_->ToLetterString(quackle);
  ASSERT_TRUE(letter_string.has_value());
  const LetterString expected(numbers, 7);
  EXPECT_EQ(letter_string.value(), expected);
}

TEST_F(TilesTest, ToString) {
  const auto quackle = tiles_->ToLetterString("QUACKLE").value();
  const absl::optional<std::string> str = tiles_->ToString(quackle);
  EXPECT_EQ("QUACKLE", tiles_->ToString(quackle).value());
}

TEST_F(TilesTest, ToStringWithBlank) {
  const auto quackle = tiles_->ToLetterString("qUACKLE").value();
  const absl::optional<std::string> str = tiles_->ToString(quackle);
  EXPECT_EQ("qUACKLE", tiles_->ToString(quackle).value());
}

TEST_F(TilesTest, Distribution) {
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('A').value()), 9);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('B').value()), 2);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('Z').value()), 1);
  EXPECT_EQ(tiles_->Count(tiles_->CharToNumber('?').value()), 2);
}

TEST_F(TilesTest, Scores) {
  EXPECT_EQ(tiles_->Score(tiles_->CharToNumber('A').value()), 1);
  EXPECT_EQ(tiles_->Score(tiles_->CharToNumber('B').value()), 3);
  EXPECT_EQ(tiles_->Score(tiles_->CharToNumber('Z').value()), 10);
  EXPECT_EQ(tiles_->Score(tiles_->CharToNumber('?').value()), 0);
}

TEST_F(TilesTest, Unblank) {
  const auto blanked = tiles_->ToLetterString("qUACkLE").value();
  const absl::optional<std::string> str = tiles_->ToString(blanked);
  const auto natural = tiles_->Unblank(blanked);
  EXPECT_EQ("QUACKLE", tiles_->ToString(natural).value());
}

TEST_F(TilesTest, PrimeIndices) {
  const auto prime_indices = tiles_->PrimeIndices();
  EXPECT_EQ(prime_indices[0], 0);  // empty

  // 12 E's is the most, it gets the lowest prime
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('E').value()], 1);

  // 9 A's and 9 I's, but A is first alphabetically, which breaks the tie.
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('A').value()], 2);
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('I').value()], 3);

  // Z is the last tile there's only one of, so it's second to last.
  // ? is forced to be last even though there's only one of it.
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('Z').value()], 26);
  EXPECT_EQ(prime_indices[tiles_->CharToNumber('?').value()], 27);
}

TEST_F(TilesTest, TilePrimes) {
  EXPECT_EQ(tiles_->Prime(0), 0);
  EXPECT_EQ(tiles_->Prime(tiles_->CharToNumber('E').value()), 2);
  EXPECT_EQ(tiles_->Prime(tiles_->CharToNumber('A').value()), 3);
  EXPECT_EQ(tiles_->Prime(tiles_->CharToNumber('I').value()), 5);

  EXPECT_EQ(tiles_->Prime(tiles_->CharToNumber('Z').value()), 101);
  EXPECT_EQ(tiles_->Prime(tiles_->CharToNumber('?').value()), 103);
}

TEST_F(TilesTest, ToProduct) {
  const LetterString ae = tiles_->ToLetterString("AE").value();
  EXPECT_EQ(tiles_->ToProduct(ae), 3 * 2);

  const LetterString aia = tiles_->ToLetterString("AIA").value();
  EXPECT_EQ(tiles_->ToProduct(aia), 3 * 5 * 3);

  // This should be the biggest product with 15 tiles.
  const LetterString ppvvwwyyjkqxz__ =
      tiles_->ToLetterString("PPVVWWYYJKQXZ??").value();
  absl::uint128 expected_product = 1;
  for (absl::uint128 p :
       {61, 61, 67, 67, 71, 71, 73, 73, 79, 83, 89, 97, 101, 103, 103}) {
    expected_product *= p;
  }
  EXPECT_EQ(tiles_->ToProduct(ppvvwwyyjkqxz__), expected_product);
}
