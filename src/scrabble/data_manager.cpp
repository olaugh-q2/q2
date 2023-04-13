#include "src/scrabble/data_manager.h"

void DataManager::LoadData(const q2::proto::DataCollection& data_collection) {
  for (const std::string& filename : data_collection.tiles_files()) {
    auto tiles = absl::make_unique<Tiles>(filename);
    tiles_.emplace(filename, std::move(tiles));
  }
  for (const std::string& filename : data_collection.board_files()) {
    auto board_layout = absl::make_unique<BoardLayout>(filename);
    board_layouts_.emplace(filename, std::move(board_layout));
  }
  for (const auto& spec : data_collection.anagram_map_file_specs()) {
    auto tiles = GetTiles(spec.tiles_filename());
    if (tiles == nullptr) {
      LOG(FATAL) << "Tiles file " << spec.tiles_filename()
                 << " not found for anagram map "
                 << spec.anagram_map_filename();
    }
    auto anagram_map =
        AnagramMap::CreateFromBinaryFile(*tiles, spec.anagram_map_filename());
    anagram_maps_.emplace(spec.anagram_map_filename(), std::move(anagram_map));
  }
}

const Tiles* DataManager::GetTiles(const std::string& filename) {
  const auto it = tiles_.find(filename);
  if (it != tiles_.end()) {
    return it->second.get();
  }
  return nullptr;
}

const BoardLayout* DataManager::GetBoardLayout(const std::string& filename) {
  const auto it = board_layouts_.find(filename);
  if (it != board_layouts_.end()) {
    return it->second.get();
  }
  return nullptr;
}

const AnagramMap* DataManager::GetAnagramMap(const std::string& filename) {
  const auto it = anagram_maps_.find(filename);
  if (it != anagram_maps_.end()) {
    return it->second.get();
  }
  return nullptr;
}

const Leaves* DataManager::GetLeaves(const std::string& filename) {
  const auto it = leaves_.find(filename);
  if (it != leaves_.end()) {
    return it->second.get();
  }
  return nullptr;
}