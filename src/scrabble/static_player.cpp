#include "src/scrabble/static_player.h"

#include "src/scrabble/move_finder.h"

Move StaticPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  SetStartOfTurnTime();
  // std::stringstream ss;
  // pos.Display(ss);
  // LOG(INFO) << "StaticPlayer::ChooseBestMove: " << std::endl << ss.str();
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

/*
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                          false);
  auto all_moves = move_finder_->Moves();
  std::sort(
      all_moves.begin(), all_moves.end(),
      [](const Move& m1, const Move& m2) { return m1.Equity() > m2.Equity(); });
  std::stringstream ss;
  pos.Display(ss);
  LOG(INFO) << "pos: " << std::endl << ss.str();
  for (const auto& m : move_finder_->Moves()) {
    if (m.Equity() > move.Equity() + 1e-5) {
      std::stringstream ss;
      pos.Display(ss);
      LOG(INFO) << "pos: " << std::endl << ss.str();
      LOG(INFO) << "number of moves: " << all_moves.size();
      for (int i = 0; i < 10; ++i) {
        if (i < all_moves.size()) {
          std::stringstream ss;
          all_moves[i].Display(tiles_, ss);
          LOG(INFO) << "move: " << ss.str() << " " << all_moves[i].Equity();
        }
      }
      LOG(INFO) << "should have found move with equity " << m.Equity()
                << " but found move with equity " << move.Equity();
    }
  }
  */
  return move;
}

void StaticPlayer::ResetGameState() { positions_with_crosses_computed_ = 0; }