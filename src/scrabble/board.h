#ifndef SRC_SCRABBLE_BOARD_H_
#define SRC_SCRABBLE_BOARD_H_

#include <array>

#include "src/scrabble/move.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Board {
 public:
  Board();
  LetterString Row(int row_index) const;
  Letter At(int row, int col) const {
    return rows_[row][col];
  }
  void UnsafePlaceMove(const Move& move);
 private:
  std::array<LetterString, 15> rows_;
};

#endif  // SRC_SCRABBLE_BOARD_H_