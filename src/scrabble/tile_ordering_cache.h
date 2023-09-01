#ifndef SRC_SCRABBLE_TILE_ORDERING_CACHE_H
#define SRC_SCRABBLE_TILE_ORDERING_CACHE_H

#include <vector>

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
            LoadRepeatableCache(config.repeatable_orderings_file())) {}

  std::vector<TileOrdering> GetTileOrderings(int game_number,
                                             int position_index,
                                             int start_index,
                                             int num_orderings) override;

 private:
  std::vector<TileOrdering> LoadRepeatableCache(const std::string& file);

  const Tiles& tiles_;
  const bool using_repeatable_cache_;
  const std::vector<TileOrdering> repeatable_cache_;
};

#endif  // SRC_SCRABBLE_TILE_ORDERING_CACHE_H