#include "src/scrabble/board.h"

#include "glog/logging.h"
#include "src/scrabble/strings.h"

Board::Board() {
  is_empty_ = true;
  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < 15; j++) {
      rows_[i].push_back(0);
    }
  }
}

LetterString Board::Row(int row_index) const {
  CHECK_GE(row_index, 0);
  CHECK_LT(row_index, 15);
  return rows_[row_index];
}

void Board::UnsafePlaceMove(const Move& move) {
  is_empty_ = false;
  int row = move.StartRow();
  int col = move.StartCol();
  for (const Letter letter : move.Letters()) {
    if (letter != 0) {  // Tiles::kUnset gives linking error ???
      rows_[row][col] = letter;
    }
    if (move.Direction() == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
}