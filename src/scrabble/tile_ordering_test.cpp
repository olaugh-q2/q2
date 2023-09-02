#include "src/scrabble/tile_ordering.h"

#include <absl/random/random.h>
#include <google/protobuf/text_format.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::google::protobuf::Arena;

std::unique_ptr<Tiles> tiles_;

class TileOrderingTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    LOG(INFO) << "tiles ok";
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
};

TEST_F(TileOrderingTest, CreateAndAdjust) {
  const std::vector<Letter> letters = {L('A'), L('Z'), L('?'), L('A')};
  const std::vector<uint16_t> dividends = {1, 2, 3, 4, 5};
  const auto ordering = TileOrdering(letters, dividends);

  EXPECT_THAT(ordering.GetLetters(),
              testing::ElementsAre(L('A'), L('Z'), L('?'), L('A')));
  EXPECT_THAT(ordering.GetExchangeInsertionDividends(),
              testing::ElementsAre(1, 2, 3, 4, 5));

  const std::vector<Letter> used_letters = {L('A'), L('Z')};
  const auto adjusted = ordering.Adjust(used_letters);

  EXPECT_THAT(adjusted.GetLetters(), testing::ElementsAre(L('A'), L('?')));
  EXPECT_THAT(adjusted.GetExchangeInsertionDividends(),
              testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST_F(TileOrderingTest, CreateFromProto) {
  Arena arena;
  auto proto = Arena::CreateMessage<q2::proto::TileOrdering>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
      letters: "AZ?A"
      exchange_insertion_dividends: 1
      exchange_insertion_dividends: 2
      exchange_insertion_dividends: 3
      exchange_insertion_dividends: 4
      exchange_insertion_dividends: 5
  )",
                                                proto);
  const auto ordering = TileOrdering(*proto, *tiles_);

  EXPECT_THAT(ordering.GetLetters(),
              testing::ElementsAre(L('A'), L('Z'), L('?'), L('A')));
  EXPECT_THAT(ordering.GetExchangeInsertionDividends(),
              testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST_F(TileOrderingTest, CreateRandom) {
  absl::BitGen gen;
  const auto ordering = TileOrdering(*tiles_, gen, 5);

  EXPECT_EQ(ordering.GetLetters().size(), 100);
  EXPECT_EQ(ordering.GetExchangeInsertionDividends().size(), 5);
}