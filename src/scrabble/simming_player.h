#ifndef SRC_SCRABBLE_SIMMING_PLAYER_H
#define SRC_SCRABBLE_SIMMING_PLAYER_H

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

class SimmingPlayer : public ComputerPlayer {
 public:
  class MoveWithResults {
   public:
    MoveWithResults()
        : move_(nullptr), iterations_(0), sum_(0), sum_of_squares_(0) {}
    MoveWithResults(const Move* move)
        : move_(move), iterations_(0), sum_(0), sum_of_squares_(0) {}
    const Move* GetMove() const { return move_; }
    int Iterations() const { return iterations_; }
    float Sum() const { return sum_; }
    float SumOfSquares() const { return sum_of_squares_; }

   private:
    const Move* move_;
    int iterations_;
    float sum_;
    float sum_of_squares_;
  };

  static void Register() {
    LOG(INFO) << "Registering SimmingPlayer";
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::SimmingPlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<SimmingPlayer>(
              dynamic_cast<const q2::proto::SimmingPlayerConfig&>(message));
        });
  }

  explicit SimmingPlayer(const q2::proto::SimmingPlayerConfig& config)
      : ComputerPlayer(config.name(), config.nickname(), config.id()),
        tiles_(*DataManager::GetInstance()->GetTiles(config.tiles_file())),
        leaves_(*(DataManager::GetInstance()->GetLeaves(config.leaves_file()))),
        num_plies_(config.plies()),
        max_plays_considered_((config.max_plays_considered() == 0)
                                  ? 999999
                                  : config.max_plays_considered()),
        static_equity_pruning_threshold_(
            config.static_equity_pruning_threshold() == 0.0
                ? 999999.9
                : config.static_equity_pruning_threshold()),
        min_iterations_(config.min_iterations()),
        max_iterations_(config.max_iterations()),
        rollout_player_(ComponentFactory::CreatePlayerFromConfig(
            config.rollout_player())) {
    move_finder_ = absl::make_unique<MoveFinder>(
        *DataManager::GetInstance()->GetAnagramMap(config.anagram_map_file()),
        *DataManager::GetInstance()->GetBoardLayout(config.board_layout_file()),
        tiles_, *DataManager::GetInstance()->GetLeaves(config.leaves_file()));
  }

  Move ChooseBestMove(const std::vector<GamePosition>* previous_position,
                      const GamePosition& position) override;

 private:
  FRIEND_TEST(SimmingPlayerTest, SelectTopN);
  FRIEND_TEST(SimmingPlayerTest, UnneededSelectTopN);
  FRIEND_TEST(SimmingPlayerTest, NoPrune);
  FRIEND_TEST(SimmingPlayerTest, SelectWithinThreshold);
  FRIEND_TEST(SimmingPlayerTest, SelectWithinThreshold2);
  FRIEND_TEST(SimmingPlayerTest, SelectTopNWithinThreshold);
  FRIEND_TEST(SimmingPlayerTest, SelectTopNWithinThreshold2);

  std::vector<Move> FindMoves(
      const std::vector<GamePosition>* previous_positions,
      const GamePosition& pos);

  std::vector<MoveWithResults> InitialPrune(const std::vector<Move>& moves);

  const Tiles& tiles_;
  int positions_with_crosses_computed_ = 0;
  const Leaves& leaves_;
  std::unique_ptr<MoveFinder> move_finder_;
  int num_plies_;

  int max_plays_considered_;
  float static_equity_pruning_threshold_;
  int min_iterations_;
  int max_iterations_;

  std::unique_ptr<ComputerPlayer> rollout_player_;
};

#endif  // SRC_SCRABBLE_SIMMING_PLAYER_H