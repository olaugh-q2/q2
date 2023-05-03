#ifndef SRC_SCRABBLE_ENDGAME_PLAYER_H
#define SRC_SCRABBLE_ENDGAME_PLAYER_H

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
#include "src/scrabble/rack.h"

class EndgamePlayer : public ComputerPlayer {
 public:
  static void Register() {
    LOG(INFO) << "Registering EndgamePlayer";
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::EndgamePlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<EndgamePlayer>(
              dynamic_cast<const q2::proto::EndgamePlayerConfig&>(message));
        });
  }

  explicit EndgamePlayer(const q2::proto::EndgamePlayerConfig& config)
      : ComputerPlayer(config.name(), config.nickname(), config.id()),
        tiles_(*DataManager::GetInstance()->GetTiles(config.tiles_file())) {
    move_finder_ = absl::make_unique<MoveFinder>(
        *DataManager::GetInstance()->GetAnagramMap(config.anagram_map_file()),
        *DataManager::GetInstance()->GetBoardLayout(config.board_layout_file()),
        tiles_, *DataManager::GetInstance()->GetLeaves(config.leaves_file()));
  }

  Move ChooseBestMove(const GamePosition& position) override;

 private:
  float GreedyEndgameEquity(const GamePosition& pos, const Move& move,
                            std::vector<Move> on_moves,
                            std::vector<Move> off_moves);
  void RemoveMovesNotOnRack(std::vector<Move>* moves, const Rack& rack);
  void RemoveBlockedMoves(std::vector<Move>* moves, const Board& board);
  float StaticEndgameEquity(const GamePosition& position,
                            const Move& move) const;
  const Tiles& tiles_;
  std::unique_ptr<MoveFinder> move_finder_;
};

#endif  // SRC_SCRABBLE_ENDGAME_PLAYER_H