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

TEST_F(TileOrderingCacheTest, CreateRandomCache) {
  Arena arena;
  auto config =
      Arena::CreateMessage<q2::proto::TileOrderingCacheConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
    tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    num_random_exchange_dividends: 3
    )",
                                                config);
  auto cache = absl::make_unique<TileOrderingCache>(*config);
  ASSERT_NE(cache, nullptr);
  ASSERT_EQ(cache->random_cache_.size(), 0);

  const auto orderings = cache->GetTileOrderings(0, 0, 0, 1);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[0].size(), 1);
  ASSERT_EQ(cache->random_cache_[0][0].size(), 1);
  ASSERT_EQ(orderings.size(), 1);
  ASSERT_EQ(orderings[0].Letters().size(), 100);
  ASSERT_EQ(orderings[0].ExchangeInsertionDividends().size(), 3);

  const auto orderings_copy = cache->GetTileOrderings(0, 0, 0, 1);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[0].size(), 1);
  ASSERT_EQ(cache->random_cache_[0][0].size(), 1);
  ASSERT_EQ(orderings_copy.size(), 1);
  ASSERT_EQ(orderings[0].Letters(), orderings_copy[0].Letters());
  ASSERT_EQ(orderings[0].ExchangeInsertionDividends(),
            orderings_copy[0].ExchangeInsertionDividends());

  const auto orderings2 = cache->GetTileOrderings(0, 0, 0, 2);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[0].size(), 1);
  ASSERT_EQ(cache->random_cache_[0][0].size(), 2);
  ASSERT_EQ(orderings2.size(), 2);
  ASSERT_EQ(orderings2[1].Letters().size(), 100);
  ASSERT_EQ(orderings2[1].ExchangeInsertionDividends().size(), 3);
  ASSERT_EQ(orderings2[0].Letters(), orderings[0].Letters());
  ASSERT_EQ(orderings2[0].ExchangeInsertionDividends(),
            orderings[0].ExchangeInsertionDividends());
  ASSERT_NE(orderings2[0].Letters(), orderings2[1].Letters());

  const auto orderings3 = cache->GetTileOrderings(0, 0, 1, 2);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[0].size(), 1);
  ASSERT_EQ(cache->random_cache_[0][0].size(), 3);
  ASSERT_EQ(orderings3.size(), 2);
  ASSERT_EQ(orderings3[0].Letters().size(), 100);
  ASSERT_EQ(orderings3[0].ExchangeInsertionDividends().size(), 3);
  ASSERT_EQ(orderings3[1].Letters().size(), 100);
  ASSERT_EQ(orderings3[1].ExchangeInsertionDividends().size(), 3);
  ASSERT_EQ(orderings3[0].Letters(), orderings2[1].Letters());
  ASSERT_EQ(orderings3[0].ExchangeInsertionDividends(),
            orderings2[1].ExchangeInsertionDividends());
  ASSERT_NE(orderings3[0].Letters(), orderings3[1].Letters());

  const auto next_position = cache->GetTileOrderings(0, 1, 0, 100);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[0].size(), 2);
  ASSERT_EQ(cache->random_cache_[0][1].size(), 100);
  ASSERT_EQ(next_position.size(), 100);

  const auto next_game = cache->GetTileOrderings(1, 0, 0, 10);
  ASSERT_EQ(cache->random_cache_.size(), 2);
  ASSERT_EQ(cache->random_cache_[1].size(), 1);
  ASSERT_EQ(cache->random_cache_[1][0].size(), 10);
  ASSERT_EQ(next_game.size(), 10);

  cache->RemoveGame(0);
  const auto next_game_still = cache->GetTileOrderings(1, 0, 0, 10);
  ASSERT_EQ(cache->random_cache_.size(), 1);
  ASSERT_EQ(cache->random_cache_[1].size(), 1);
  ASSERT_EQ(cache->random_cache_[1][0].size(), 10);
  ASSERT_EQ(next_game_still.size(), 10);

}
