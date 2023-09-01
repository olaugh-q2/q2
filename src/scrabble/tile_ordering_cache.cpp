#include "src/scrabble/tile_ordering_cache.h"

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
  LOG(ERROR) << "fresh random cache not implemented yet";
  return {};
}
