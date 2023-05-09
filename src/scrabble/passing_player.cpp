#include "src/scrabble/passing_player.h"

Move PassingPlayer::ChooseBestMove(const std::vector<GamePosition>* previous_positions,
                                   const GamePosition& position) {
  SetStartOfTurnTime();
  Move move;
  move.SetScore(0);
  return move;
}