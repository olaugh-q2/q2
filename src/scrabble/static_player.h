#ifndef SRC_SCRABBLE_STATIC_PLAYER_H
#define SRC_SCRABBLE_STATIC_PLAYER_H

#include "absl/memory/memory.h"
#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"
#include "src/scrabble/move_finder.h"

class StaticPlayer : public ComputerPlayer {
 public:
  StaticPlayer(int id, const AnagramMap& anagram_map, const BoardLayout& layout,
               const Tiles& tiles, const Leaves& leaves)
      : ComputerPlayer("Static Player", "static", id) {
    move_finder_ =
        absl::make_unique<MoveFinder>(anagram_map, layout, tiles, leaves);
  }

  Move ChooseBestMove(const GamePosition& position) override;

 private:
  std::unique_ptr<MoveFinder> move_finder_;
};

#endif  // SRC_SCRABBLE_STATIC_PLAYER_H