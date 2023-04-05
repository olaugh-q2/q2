#include "src/scrabble/game_position.h"

void GamePosition::Display(std::ostream& os) const {
  layout_.DisplayBoard(board_, tiles_, os);
  if (move_) {
    os << "Player " << on_turn_player_id_ << " holding ";
    rack_.Display(tiles_, os);
    os << " on " << player_score_ << " to opp's " << opponent_score_;
    os << " plays ";
    move_->Display(tiles_, os);
    os << std::endl;
    os << "Unseen (before drawing): ";
  } else {
    os << "Player " << on_turn_player_id_ << " holds ";
    rack_.Display(tiles_, os);
    os << " on " << player_score_ << " to opp's " << opponent_score_
       << std::endl;
    os << "Unseen: ";
  }
  unseen_to_player_.Display(os);
  os << std::endl;
}