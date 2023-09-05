#include "src/scrabble/tile_ordering_cache.h"

#include <absl/random/random.h>
#include <fcntl.h>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "src/scrabble/computer_player.h"

using ::google::protobuf::Arena;
using ::google::protobuf::TextFormat;
using ::google::protobuf::io::FileInputStream;

std::vector<TileOrdering> TileOrderingCache::LoadRepeatableCache(
    const std::string& file) {
  if (!using_repeatable_cache_) {
    return {};
  }
  Arena arena;
  auto tile_orderings = Arena::CreateMessage<q2::proto::TileOrderings>(&arena);
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    LOG(ERROR) << "Error opening TileOrderings file " << file;
    return {};
  }

  google::protobuf::io::FileInputStream fis(fd);
  fis.SetCloseOnDelete(true);

  if (!google::protobuf::TextFormat::Parse(&fis, tile_orderings)) {
    LOG(ERROR) << "Failed to parse TileOrderings file " << file;
    return {};
  }

  std::vector<TileOrdering> ret;
  ret.reserve(tile_orderings->tile_orderings_size());
  for (const auto& ordering : tile_orderings->tile_orderings()) {
    ret.emplace_back(ordering, tiles_);
  }
  return ret;
}

std::vector<TileOrdering> TileOrderingCache::GetTileOrderings(
    int game_number, int position_index, int start_index, int num_orderings) {
  if (using_repeatable_cache_) {
    CHECK_LE(start_index + num_orderings, repeatable_cache_.size());
    return std::vector<TileOrdering>(
        repeatable_cache_.begin() + start_index,
        repeatable_cache_.begin() + start_index + num_orderings);
  }

  std::lock_guard<std::mutex> lock(random_cache_mutex_);

  int num_orderings_still_needed = num_orderings;
  std::vector<TileOrdering> ret;
  auto game_it = random_cache_.find(game_number);
  if (game_it != random_cache_.end()) {
    auto pos_it = game_it->second.find(position_index);
    if (pos_it != game_it->second.end()) {
      if (pos_it->second.size() >= start_index + num_orderings) {
        // Cache hit, read from random_cache_
        return std::vector<TileOrdering>(
            pos_it->second.begin() + start_index,
            pos_it->second.begin() + start_index + num_orderings);
      } else {
        ret = std::vector<TileOrdering>(
            pos_it->second.begin() + start_index, pos_it->second.end());
        // Cache hit, but not enough orderings, so generate more below
        num_orderings_still_needed -= pos_it->second.size() - start_index;
      }
    }
  }

  absl::BitGen gen;
  for (int i = 0; i < num_orderings_still_needed; ++i) {
    ret.emplace_back(tiles_, gen, num_random_exchange_dividends_);
    random_cache_[game_number][position_index].push_back(ret.back());
  }
  return ret;
}
