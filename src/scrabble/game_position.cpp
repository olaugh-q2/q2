#include "src/scrabble/game_position.h"

void GamePosition::Display(std::ostream& os) const {
  const Bag bag(tiles_);
  Display(os, bag);
}

void GamePosition::Display(std::ostream& os, const Bag& initial_bag) const {
  //LOG(INFO) << "GamePosition::Display(...)";
  layout_.DisplayBoard(board_, tiles_, os);
  if (move_) {
    if (scoreless_turns_ > 0) {
      os << "[Consecutive scoreless turns preceding this position: "
         << scoreless_turns_ << "]" << std::endl;
      if (!IsScorelessTurn()) {
        os << "This move breaks the streak." << std::endl;
      } else {
        os << "This move makes it " << scoreless_turns_ + 1
           << " consecutive scoreless turns." << std::endl;
      }
    }
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
    if (scoreless_turns_ > 0) {
      os << "[Consecutive scoreless turns preceding this position: "
         << scoreless_turns_ << "]" << std::endl;
    }
    os << "Player " << on_turn_player_id_ << " holds ";
    rack_.Display(tiles_, os);
    os << " on " << player_score_ << " to opp's " << opponent_score_;

    int minutes = absl::ToInt64Minutes(time_remaining_start_);
    int seconds = absl::ToInt64Seconds(time_remaining_start_) % 60;
    os << " [" << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << "]"
       << std::endl;

    os << "Unseen: ";
  }
  if (known_opp_rack_.NumTiles() > 0) {
    known_opp_rack_.Display(tiles_, os);
  } else {
    //os << "[temporarily disabled]";    
    auto unseen_to_player = GetUnseenToPlayer(initial_bag);
    unseen_to_player.Display(os);
  }
  os << std::endl;
}

bool GamePosition::IsScorelessTurn() const {
  CHECK(move_.has_value())
      << "Should not call IsScorelessTurn() before move is committed.";
  // Move::Exchange includes passes
  return move_->GetAction() == Move::Exchange;
}

void GamePosition::WriteProto(q2::proto::GamePosition* proto) const {
  proto->set_on_turn_player_id(on_turn_player_id_);
  proto->set_opponent_player_id(opponent_player_id_);
  proto->set_rack(tiles_.ToString(rack_.Letters()).value());
  proto->set_player_score(player_score_);
  proto->set_opponent_score(opponent_score_);
  proto->set_position_index(position_index_);
  proto->set_time_remaining_start_micros(
      absl::ToInt64Microseconds(time_remaining_start_));
  proto->set_scoreless_turns(scoreless_turns_);
  if (move_.has_value()) {
    move_->WriteProto(tiles_, proto->mutable_move());
    board_.WriteMoveWords(*move_, tiles_, proto->mutable_move());
  }
}

GamePosition GamePosition::SwapRacks() const {
  LOG(INFO) << "GamePosition::SwapRacks()";
  const auto unseen = GetUnseenToPlayer();
  CHECK_LE(unseen.Size(), 7);
  LetterString rack;
  for (const auto& letter : unseen.Letters()) {
    rack.push_back(letter);
  }
  return GamePosition(layout_, board_, opponent_player_id_, on_turn_player_id_,
                      rack, opponent_score_, player_score_, position_index_,
                      game_index_, time_remaining_start_, scoreless_turns_,
                      tiles_);
}