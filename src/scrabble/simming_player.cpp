#include "src/scrabble/simming_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move SimmingPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  SetStartOfTurnTime();
  auto all_moves = FindMoves(previous_positions, pos);
  std::sort(
      all_moves.begin(), all_moves.end(),
      [](const Move& m1, const Move& m2) { return m1.Equity() > m2.Equity(); });
  return all_moves[0];
}

std::vector<Move> SimmingPlayer::FindMoves(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  std::stringstream ss;
  pos.Display(ss);
  LOG(INFO) << "SimmingPlayer::FindMoves: " << std::endl << ss.str();
  CHECK_NE(previous_positions, nullptr);
  // LOG(INFO) << "positions_with_crosses_computed_: "
  //            << positions_with_crosses_computed_;
  if (positions_with_crosses_computed_ == 0) {
    move_finder_->ClearHookTables();
  }
  for (int i = positions_with_crosses_computed_ + 1;
       i < previous_positions->size(); ++i) {
    // LOG(INFO) << "i: " << i;
    const auto& p1 = (*previous_positions)[i - 1];
    const auto& p2 = (*previous_positions)[i];
    move_finder_->CacheCrossesAndScores(p2.GetBoard(), *p1.GetMove());
    positions_with_crosses_computed_++;
  }
  // LOG(INFO) << "positions_with_crosses_computed_: "
  //           << positions_with_crosses_computed_;
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordBest,
                          false);
  const auto& moves = move_finder_->Moves();
  CHECK_EQ(moves.size(), 1);
  auto move = moves[0];

  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                          false);
  return move_finder_->Moves();
}

std::vector<SimmingPlayer::MoveWithResults> SimmingPlayer::InitialPrune(
    const std::vector<Move>& moves) {
  std::vector<SimmingPlayer::MoveWithResults> ret;
  ret.reserve(moves.size());
  for (const auto& move : moves) {
    ret.emplace_back(&move);
  }
  const size_t new_size =
      std::min(ret.size(), static_cast<size_t>(max_plays_considered_));
  std::partial_sort(ret.begin(), ret.begin() + new_size, ret.end(),
                    [](const MoveWithResults& a, const MoveWithResults& b) {
                      return a.GetMove()->Equity() > b.GetMove()->Equity();
                    });

  CHECK_GE(ret.size(), 1);
  const float best_equity = ret[0].GetMove()->Equity();
  // Find the first move that should be pruned.
  auto it = std::find_if(ret.begin(), ret.end(),
                         [this, best_equity](const MoveWithResults& a) {
                           return (best_equity - a.GetMove()->Equity()) >
                                  static_equity_pruning_threshold_;
                         });

  // Erase that move and all moves after it.
  if (it != ret.end()) {
    ret.erase(it, ret.end());
  }

  // Resize to the number of max plays considered, if needed.
  ret.resize(std::min(ret.size(), new_size));
  return ret;
}