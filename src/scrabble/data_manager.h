#ifndef SRC_SCRABBLE_DATA_MANAGER_H
#define SRC_SCRABBLE_DATA_MANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/tiles.h"

class DataManager {
 public:
  static DataManager* GetInstance() {
    static DataManager instance;
    return &instance;
  }
  void LoadData(const q2::proto::DataCollection& data_collection);
  const AnagramMap* GetAnagramMap(const std::string& filename);
  const BoardLayout* GetBoardLayout(const std::string& filename);
  const Tiles* GetTiles(const std::string& filename);
  const Leaves* GetLeaves(const std::string& filename);

 private:
  DataManager() = default;
  std::unordered_map<std::string, std::unique_ptr<AnagramMap>> anagram_maps_;
  std::unordered_map<std::string, std::unique_ptr<BoardLayout>> board_layouts_;
  std::unordered_map<std::string, std::unique_ptr<Tiles>> tiles_;
  std::unordered_map<std::string, std::unique_ptr<Leaves>> leaves_;
};

#endif  // SRC_SCRABBLE_DATA_MANAGER_H
