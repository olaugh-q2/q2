#ifndef SRC_SCRABBLE_PASSING_PLAYER_H
#define SRC_SCRABBLE_PASSING_PLAYER_H

#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"

class PassingPlayer : public ComputerPlayer {
 public:
  static void Register() {
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::PassingPlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<PassingPlayer>(
              dynamic_cast<const q2::proto::PassingPlayerConfig&>(message));
        });
  }

  explicit PassingPlayer(int id)
      : ComputerPlayer("Passing Player", "passing", id) {}

  explicit PassingPlayer(const q2::proto::PassingPlayerConfig& config)
      : ComputerPlayer(config.name(), config.nickname(), config.id()) {}
  Move ChooseBestMove(const std::vector<GamePosition>* previous_positions,
                      const GamePosition& position) override;
};

#endif  // SRC_SCRABBLE_PASSING_PLAYER_H