#include "src/scrabble/tile_ordering_cache.h"

#include <google/protobuf/text_format.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::google::protobuf::Arena;

class TileOrderingCacheTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    DataManager* dm = DataManager::GetInstance();
    Arena arena;
    auto spec = Arena::CreateMessage<q2::proto::DataCollection>(&arena);
    google::protobuf::TextFormat::ParseFromString(R"(
      tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
      board_files: "src/scrabble/testdata/scrabble_board.textproto"
    )",
                                                  spec);
    dm->LoadData(*spec);
  }

  Letter L(char c) {
    DataManager* dm = DataManager::GetInstance();
    const Tiles* tiles =
        dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
    return tiles->CharToNumber(c).value();
  }

  std::vector<Letter> LV(const std::string& s) {
    std::vector<Letter> ret;
    ret.reserve(s.size());
    for (const char c : s) {
      ret.push_back(L(c));
    }
    return ret;
  }
};

TEST_F(TileOrderingCacheTest, CreateRepeatableCache) {
  Arena arena;
  auto config =
      Arena::CreateMessage<q2::proto::TileOrderingCacheConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
    tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    repeatable_orderings_file: "src/scrabble/testdata/small_repeatable_orderings.textproto"
    )",
                                                config);
  auto cache = absl::make_unique<TileOrderingCache>(*config);
  ASSERT_NE(cache, nullptr);

  const auto orderings = cache->GetTileOrderings(0, 0, 0, 1);
  ASSERT_EQ(orderings.size(), 1);
  EXPECT_EQ(orderings[0].Letters(), LV("ABCDEFG??"));
  EXPECT_THAT(orderings[0].ExchangeInsertionDividends(),
              testing::ElementsAre(0, 1, 2));

  const auto orderings2 = cache->GetTileOrderings(0, 0, 1, 2);
  ASSERT_EQ(orderings2.size(), 2);
  EXPECT_EQ(orderings2[0].Letters(), LV("ABCD??EFG"));
  EXPECT_THAT(orderings2[0].ExchangeInsertionDividends(),
              testing::ElementsAre(2, 2, 2));
  EXPECT_EQ(orderings2[1].Letters(), LV("??ABCDEFG"));
  EXPECT_THAT(orderings2[1].ExchangeInsertionDividends(),
              testing::ElementsAre(3, 2, 1));

  const auto orderings3 = cache->GetTileOrderings(0, 0, 0, 3);
  ASSERT_EQ(orderings3.size(), 3);
  EXPECT_EQ(orderings3[0].Letters(), LV("ABCDEFG??"));
  EXPECT_THAT(orderings3[0].ExchangeInsertionDividends(),
              testing::ElementsAre(0, 1, 2));
  EXPECT_EQ(orderings3[1].Letters(), LV("ABCD??EFG"));
  EXPECT_THAT(orderings3[1].ExchangeInsertionDividends(),
              testing::ElementsAre(2, 2, 2));
  EXPECT_EQ(orderings3[2].Letters(), LV("??ABCDEFG"));
  EXPECT_THAT(orderings3[2].ExchangeInsertionDividends(),
              testing::ElementsAre(3, 2, 1));

}

/*
TEST_F(TileOrderingCacheTest, CreateRandomCache) {
  Arena arena;
  auto config =
      Arena::CreateMessage<q2::proto::TileOrderingCacheConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
    tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    )",
                                                config);
  auto cache = absl::make_unique<TileOrderingCache>(*config);
  ASSERT_NE(cache, nullptr);

  const auto orderings = cache->GetTileOrderings(0, 0, 0, 1);
  ASSERT_EQ(orderings.size(), 1);
  EXPECT_EQ(orderings[0].Letters(), LV("ABCDEFG??"));
}
*/