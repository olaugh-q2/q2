#ifndef SRC_SCRABBLE_TILE_ORDERING_CACHE_H
#define SRC_SCRABBLE_TILE_ORDERING_CACHE_H

#include <mutex>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/data_manager.h"
#include "src/scrabble/tile_ordering.h"

class TileOrderingCache : public TileOrderingProvider {
 public:
  static void Register() {
    ComponentFactory::GetInstance()->RegisterTileOrderingProvider(
        q2::proto::TileOrderingCacheConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<TileOrderingCache>(
              dynamic_cast<const q2::proto::TileOrderingCacheConfig&>(message));
        });
  }

  explicit TileOrderingCache(const q2::proto::TileOrderingCacheConfig& config)
      : tiles_(*DataManager::GetInstance()->GetTiles(config.tiles_file())),
        using_repeatable_cache_(config.repeatable_orderings_file().size() > 0),
        repeatable_cache_(
            LoadRepeatableCache(config.repeatable_orderings_file())),
        num_random_exchange_dividends_(config.num_random_exchange_dividends()) {
  }

  std::vector<TileOrdering> GetTileOrderings(int game_number,
                                             int position_index,
                                             int start_index,
                                             int num_orderings) override;

  void RemoveGame(int game_number) override {
    std::lock_guard<std::mutex> lock(random_cache_mutex_);
    random_cache_.erase(game_number);
  }                                             

 private:
  FRIEND_TEST(TileOrderingCacheTest, CreateRandomCache);

  std::vector<TileOrdering> LoadRepeatableCache(const std::string& file);

  const Tiles& tiles_;
  const bool using_repeatable_cache_;
  const std::vector<TileOrdering> repeatable_cache_;
  const int num_random_exchange_dividends_;
  absl::flat_hash_map<int, absl::flat_hash_map<int, std::vector<TileOrdering>>>
      random_cache_;
  std::mutex random_cache_mutex_;
};

#endif  // SRC_SCRABBLE_TILE_ORDERING_CACHE_H