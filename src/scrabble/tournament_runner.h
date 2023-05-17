#ifndef SRC_SCRABBLE_TOURNAMENT_RUNNER_H
#define SRC_SCRABBLE_TOURNAMENT_RUNNER_H

#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"

class TournamentRunner {
 public:
  explicit TournamentRunner(const q2::proto::TournamentSpec& spec)
      : spec_(spec) {}
  void RunGames(const Tiles& tiles, const BoardLayout& board_layout,
                int thread_index, int start_index, int num_pairs,
                std::vector<std::unique_ptr<q2::proto::PlayerResults>>*
                    player_results) const;
  void ComputeAverages(
      const std::vector<std::unique_ptr<q2::proto::PlayerResults>>& results,
      std::vector<std::unique_ptr<q2::proto::PlayerAverages>>* averages) const;
  void Run(q2::proto::TournamentResults* results);

 private:
  const q2::proto::TournamentSpec& spec_;
  std::vector<std::unique_ptr<ComputerPlayer>> players_;
};

#endif  // SRC_SCRABBLE_TOURNAMENT_RUNNER_H
