#include "src/scrabble/board.h"

#include "glog/logging.h"
#include "src/scrabble/strings.h"

Board::Board() {
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