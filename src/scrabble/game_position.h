#ifndef SRC_SCRABBLE_GAME_POSITION_H_
#define SRC_SCRABBLE_GAME_POSITION_H_

#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/rack.h"

class GamePosition {
 public:
  GamePosition(const BoardLayout& layout, const Board& board, int on_turn_player_id,
               int opponent_player_id, const Rack& rack, int player_score,
               int opponent_score, const GamePosition* previous_position,
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
        tiles_(tiles) {}

  void CommitMove(const Move& move) {
    move_ = move;
  }

  void Display(std::ostream& os) const;

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
  const Tiles& tiles_;
};

#endif  // SRC_SCRABBLE_GAME_POSITION_H_