#include "src/scrabble/static_player.h"

#include "src/scrabble/move_finder.h"

Move StaticPlayer::ChooseBestMove(const GamePosition& pos) {
  SetStartOfTurnTime();
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordBest);
  const auto& moves = move_finder_->Moves();
  CHECK_EQ(moves.size(), 1);
  return moves[0];
}