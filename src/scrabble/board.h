#ifndef SRC_SCRABBLE_BOARD_H_
#define SRC_SCRABBLE_BOARD_H_

#include <array>

#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Board {
 public:
  Board();
  LetterString Row(int row_index) const;
 private:
  std::array<LetterString, 15> rows_;
};

#endif  // SRC_SCRABBLE_BOARD_H_