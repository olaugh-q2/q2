#ifndef SRC_SCRABBLE_STATIC_PLAYER_H
#define SRC_SCRABBLE_STATIC_PLAYER_H

#include "absl/memory/memory.h"
#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/data_manager.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"
#include "src/scrabble/move_finder.h"

class StaticPlayer : public ComputerPlayer {
 public:
  static void Register() {
    LOG(INFO) << "Registering StaticPlayer";
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::StaticPlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<StaticPlayer>(
              dynamic_cast<const q2::proto::StaticPlayerConfig&>(message));
        });
  }

  StaticPlayer(int id, const AnagramMap& anagram_map, const BoardLayout& layout,
               const Tiles& tiles, const Leaves& leaves)
      : ComputerPlayer("Static Player", "static", id), tiles_(tiles) {
    move_finder_ =
        absl::make_unique<MoveFinder>(anagram_map, layout, tiles, leaves);
    positions_with_crosses_computed_ = 0;
  }

  explicit StaticPlayer(const q2::proto::StaticPlayerConfig& config)
      : StaticPlayer(
            config.id(),
            *DataManager::GetInstance()->GetAnagramMap(
                config.anagram_map_file()),
            *DataManager::GetInstance()->GetBoardLayout(
                config.board_layout_file()),
            *DataManager::GetInstance()->GetTiles(config.tiles_file()),
            *DataManager::GetInstance()->GetLeaves(config.leaves_file())) {}
  Move ChooseBestMove(const std::vector<GamePosition>* previous_positions,
                      const GamePosition& position) override;
  void ResetGameState() override;

 private:
  std::unique_ptr<MoveFinder> move_finder_;
  int positions_with_crosses_computed_ = 0;
  const Tiles& tiles_;
};

#endif  // SRC_SCRABBLE_STATIC_PLAYER_H