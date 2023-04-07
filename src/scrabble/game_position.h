#ifndef SRC_SCRABBLE_GAME_POSITION_H
#define SRC_SCRABBLE_GAME_POSITION_H

#include "absl/time/time.h"
#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/rack.h"

class GamePosition {
 public:
  GamePosition(const BoardLayout& layout, const Board& board, int on_turn_player_id,
               int opponent_player_id, const Rack& rack, int player_score,
               int opponent_score, const GamePosition* previous_position,
               absl::Duration time_remaining_start,
               const Tiles& tiles)
      : layout_(layout),
        board_(board),
        on_turn_player_id_(on_turn_player_id),
        opponent_player_id_(opponent_player_id),
        rack_(rack),
        player_score_(player_score),
        opponent_score_(opponent_score),
        previous_position_(previous_position),
        unseen_to_player_(*Bag::UnseenToPlayer(tiles, board_, rack_)),
        time_remaining_start_(time_remaining_start),
        tiles_(tiles) {}

  void CommitMove(const Move& move, const absl::Duration elapsed) {
    move_ = move;
    CHECK_GE(elapsed, absl::ZeroDuration());
    time_remaining_end_ = time_remaining_start_ - elapsed;
  }

  void Display(std::ostream& os) const;

  const Board& GetBoard() const { return board_; }
  const Rack& GetRack() const { return rack_; }
  const Bag& GetUnseenToPlayer() const { return unseen_to_player_; }
  const absl::optional<Move>& GetMove() const { return move_; }
 private:
  const BoardLayout& layout_;
  Board board_;
  int on_turn_player_id_;
  int opponent_player_id_;

  // Rack for on-turn player.
  Rack rack_;

  // Score for on-turn player.
  int player_score_;

  // Score for opponent player.
  int opponent_score_;

  // nullptr if this is the first position.
  const GamePosition* previous_position_;

  // Move played in this position. nullopt if not yet played.
  absl::optional<Move> move_;

  const Bag unseen_to_player_;

  // Time left in game for player at start of turn (possibly negative)
  const absl::Duration time_remaining_start_; 

  // Time left in game for player at end of turn (possibly negative)
  absl::Duration time_remaining_end_;

  const Tiles& tiles_;
};

#endif  // SRC_SCRABBLE_GAME_POSITION_H