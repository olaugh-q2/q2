#ifndef SRC_SCRABBLE_GAME_POSITION_H
#define SRC_SCRABBLE_GAME_POSITION_H

#include "absl/time/time.h"
#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/rack.h"

class GamePosition {
 public:
  GamePosition(const BoardLayout& layout, const Board& board,
               int on_turn_player_id, int opponent_player_id, const Rack& rack,
               int player_score, int opponent_score, int position_index,
               absl::Duration time_remaining_start, int scoreless_turns,
               const Tiles& tiles)
      : layout_(layout),
        board_(board),
        on_turn_player_id_(on_turn_player_id),
        opponent_player_id_(opponent_player_id),
        rack_(rack),
        player_score_(player_score),
        opponent_score_(opponent_score),
        position_index_(position_index),
        time_remaining_start_(time_remaining_start),
        scoreless_turns_(scoreless_turns),
        tiles_(tiles) {}

  void CommitMove(const Move& move, const absl::Duration elapsed) {
    move_ = move;
    CHECK_GE(elapsed, absl::ZeroDuration());
    time_remaining_end_ = time_remaining_start_ - elapsed;
  }

  void Display(std::ostream& os) const;
  void Display(std::ostream& os, const Bag& initial_bag) const;
  const Board& GetBoard() const { return board_; }
  const Rack& GetRack() const { return rack_; }
  Bag GetUnseenToPlayer() const {
    Bag bag(tiles_);
    return bag.UnseenToPlayer(board_, rack_);
  }
  Bag GetUnseenToPlayer(const Bag& bag) const {
    return bag.UnseenToPlayer(board_, rack_);
  }
  const absl::optional<Move>& GetMove() const { return move_; }
  bool IsScorelessTurn() const;
  int OnTurnPlayerId() const { return on_turn_player_id_; }
  int OpponentPlayerId() const { return opponent_player_id_; }
  int PlayerScore() const { return player_score_; }
  int OpponentScore() const { return opponent_score_; }
  std::size_t PositionIndex() const { return position_index_; }
  absl::Duration TimeRemainingEnd() const { return time_remaining_end_; }
  int ScorelessTurns() const { return scoreless_turns_; }

 private:
  const BoardLayout& layout_;
  Board board_;
  int on_turn_player_id_;
  int opponent_player_id_;

  // Rack for on-turn player.
  const Rack rack_;

  // Score for on-turn player.
  const int player_score_;

  // Score for opponent player.
  const int opponent_score_;

  const std::size_t position_index_;

  // Move played in this position. nullopt if not yet played.
  absl::optional<Move> move_;

  // Time left in game for player at start of turn (possibly negative)
  const absl::Duration time_remaining_start_;

  // Time left in game for player at end of turn (possibly negative)
  absl::Duration time_remaining_end_;

  // Previous consecutive scoreless turns. Does not include this turn.
  const int scoreless_turns_;

  const Tiles& tiles_;

};

#endif  // SRC_SCRABBLE_GAME_POSITION_H