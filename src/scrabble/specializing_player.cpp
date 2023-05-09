#include "src/scrabble/specializing_player.h"

#include "src/scrabble/computer_player.h"
#include "src/scrabble/predicate.h"

Move SpecializingPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  SetStartOfTurnTime();
  for (int i = 0; i < predicates_.size(); ++i) {
    if (predicates_[i]->Evaluate(pos)) {
      return players_[i]->ChooseBestMove(previous_positions, pos);
    }
  }
  LOG(ERROR) << "No predicate matched for player " << Name() << " at position:";
  std::stringstream ss;
  pos.Display(ss);
  LOG(INFO) << ss.str();
  // Return pass.
  Move move;
  move.SetScore(0);
  return move;
}

void SpecializingPlayer::ResetGameState() {
  for (auto& player : players_) {
    player->ResetGameState();
  }
}