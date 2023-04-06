#include "src/scrabble/game_position.h"

void GamePosition::Display(std::ostream& os) const {
  layout_.DisplayBoard(board_, tiles_, os);
  if (move_) {
    os << "Player " << on_turn_player_id_ << " holding ";
    rack_.Display(tiles_, os);
    os << " on " << player_score_ << " to opp's " << opponent_score_;

    os << " plays ";
    move_->Display(tiles_, os);

    int minutes0 = absl::ToInt64Minutes(time_remaining_start_);
    int seconds0 = absl::ToInt64Seconds(time_remaining_start_) % 60;
    os << " [" << minutes0 << ":" << (seconds0 < 10 ? "0" : "") << seconds0
       << " → ";
    int minutes1 = absl::ToInt64Minutes(time_remaining_end_);
    int seconds1 = absl::ToInt64Seconds(time_remaining_end_) % 60;
    os << minutes1 << ":" << (seconds1 < 10 ? "0" : "") << seconds1 << "]"
       << std::endl;

    os << "Unseen (before drawing): ";
  } else {
    os << "Player " << on_turn_player_id_ << " holds ";
    rack_.Display(tiles_, os);
    os << " on " << player_score_ << " to opp's " << opponent_score_;

    int minutes = absl::ToInt64Minutes(time_remaining_start_);
    int seconds = absl::ToInt64Seconds(time_remaining_start_) % 60;
    os << " [" << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << "]"
       << std::endl;

    os << "Unseen: ";
  }
  unseen_to_player_.Display(os);
  os << std::endl;
}