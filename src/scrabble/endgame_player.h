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
  class MoveWithDelta {
   public:
    MoveWithDelta(const Move* move)
        : move_(move), score_(move->Score()), equity_(move->Equity()) {}
    const Move* GetMove() const { return move_; }
    void Nullify() { move_ = nullptr; }
    void SetScore(int score) { score_ = score; }
    void SetEquity(float equity) { equity_ = equity; }
    int Score() const { return score_; }
    float Equity() const { return equity_; }

   private:
    const Move* move_;
    int score_;
    float equity_;
  };

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
    for (int i = 0; i < 2; i++) {
      move_finders_.push_back(absl::make_unique<MoveFinder>(
          *DataManager::GetInstance()->GetAnagramMap(config.anagram_map_file()),
          *DataManager::GetInstance()->GetBoardLayout(
              config.board_layout_file()),
          tiles_,
          *DataManager::GetInstance()->GetLeaves(config.leaves_file())));
    }
  }

  Move ChooseBestMove(const std::vector<GamePosition>* previous_position,
                      const GamePosition& position) override;

 private:
  float GreedyEndgameEquity(const GamePosition& pos, const MoveWithDelta& move,
                            std::vector<MoveWithDelta> on_moves,
                            std::vector<MoveWithDelta> off_moves);
  float StaticEndgameEquity(const GamePosition& position,
                            const Move& move) const;
  const Tiles& tiles_;
  std::vector<std::unique_ptr<MoveFinder>> move_finders_;
};

#endif  // SRC_SCRABBLE_ENDGAME_PLAYER_H