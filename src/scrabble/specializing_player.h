#ifndef SRC_SCRABBLE_SPECIALIZING_PLAYER_H
#define SRC_SCRABBLE_SPECIALIZING_PLAYER_H

#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"
#include "src/scrabble/predicate.h"

class SpecializingPlayer : public ComputerPlayer {
 public:
  static void Register() {
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::SpecializingPlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<SpecializingPlayer>(
              dynamic_cast<const q2::proto::SpecializingPlayerConfig&>(
                  message));
        });
  }

  explicit SpecializingPlayer(const q2::proto::SpecializingPlayerConfig& config)
  : ComputerPlayer(config.name(), config.nickname(), config.id()) {
    for (const auto& conditional_player : config.conditional_players()) {
      players_.emplace_back(ComponentFactory::CreatePlayerFromConfig(
          conditional_player.player()));
      predicates_.emplace_back(ComponentFactory::CreatePredicateFromConfig(
          conditional_player.predicate()));
    }
  }

  Move ChooseBestMove(const GamePosition& position) override;

  private:
    std::vector<std::unique_ptr<Predicate>> predicates_;
    std::vector<std::unique_ptr<ComputerPlayer>> players_;
};

#endif  // SRC_SCRABBLE_SPECIALIZING_PLAYER_H