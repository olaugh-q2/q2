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

LetterString Board::MainWord(const Move& move, const Tiles& tiles) const {
  LetterString ret = move.Letters();
  int row = move.StartRow();
  int col = move.StartCol();
  for (int i = 0; i < move.Letters().size(); ++i) {
    if (move.Letters()[i] == 0) {
      ret[i] = At(row, col);
    }
    if (ret[i] > tiles.BlankIndex()) {
      ret[i] -= tiles.BlankIndex();
    }
    if (move.Direction() == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  return ret;
}

absl::optional<LetterString> Board::CrossAt(const Move& move,
                                            const Tiles& tiles, int square_row,
                                            int square_col) const {
  // LOG(INFO) << "CrossAt(" << square_row << ", " << square_col << ")";
  CHECK_EQ(At(square_row, square_col), 0);
  int sq_pos_in_word =
      std::max(square_row - move.StartRow(), square_col - move.StartCol());
  const Letter sq_letter = move.Letters()[sq_pos_in_word];
  CHECK_NE(sq_letter, 0);
  LetterString ret;
  int row = square_row;
  int col = square_col;
  if (move.Direction() == Move::Across) {
    row--;
  } else {
    col--;
  }
  int spots_before = 0;
  while (row >= 0 && col >= 0) {
    // LOG(INFO) << "  row: " << row << ", col: " << col;
    Letter letter = At(row, col);
    if (letter) {
      spots_before++;
    } else {
      break;
    }
    if (move.Direction() == Move::Across) {
      row--;
    } else {
      col--;
    }
  }
  // LOG(INFO) << "  spots_before: " << spots_before;
  row = square_row;
  col = square_col;
  if (move.Direction() == Move::Across) {
    row -= spots_before;
  } else {
    col -= spots_before;
  }

  for (int i = 0; i < spots_before; ++i) {
    // LOG(INFO) << "  row: " << row << ", col: " << col;
    Letter letter = At(row, col);
    // LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    ret.push_back(letter);
    if (move.Direction() == Move::Across) {
      row++;
    } else {
      col++;
    }
  }
  ret.push_back(sq_letter);
  row = square_row;
  col = square_col;
  if (move.Direction() == Move::Across) {
    row++;
  } else {
    col++;
  }
  while (row < 15 && col < 15) {
    Letter letter = At(row, col);
    // LOG(INFO) << "  row: " << row << ", col: " << col;
    // LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    if (letter) {
      ret.push_back(letter);
    } else {
      break;
    }
    if (move.Direction() == Move::Across) {
      row++;
    } else {
      col++;
    }
  }
  if (ret.length() > 1) {
    for (Letter& letter : ret) {
      if (letter > tiles.BlankIndex()) {
        letter -= tiles.BlankIndex();
      }
    }
    return ret;
  } else {
    return absl::nullopt;
  }
}

std::vector<std::string> Board::CrossWords(const Move& move,
                                           const Tiles& tiles) const {
  std::vector<std::string> words;
  if (move.GetAction() != Move::Place) {
    return words;
  }
  const int row = move.StartRow();
  const int col = move.StartCol();
  for (int i = 0; i < move.Letters().length(); ++i) {
    const int square_row = row + (move.Direction() == Move::Across ? 0 : i);
    const int square_col = col + (move.Direction() == Move::Across ? i : 0);
    if (At(square_row, square_col) == 0) {
      const auto cross = CrossAt(move, tiles, square_row, square_col);
      if (cross) {
        words.push_back(tiles.ToString(cross.value()).value());
      }
    }
  }
  return words;
}

void Board::WriteMoveWords(const Move& move, const Tiles& tiles,
                           q2::proto::Move* proto) const {
  if (move.GetAction() != Move::Place) {
    return;
  }
  const LetterString main_word = MainWord(move, tiles);
  proto->add_all_words_formed(tiles.ToString(main_word).value());
  for (const std::string& word : CrossWords(move, tiles)) {
    proto->add_all_words_formed(word);
  }
}

void Board::SetLetters(const std::vector<std::string>& rows,
                       const Tiles& tiles) {
  for (int i = 0; i < 15; ++i) {
    const int src_i = i + 2;  // two header rows
    for (int j = 0; j < 15; ++j) {
      const int src_j = j * 2 + 3; // two digits and a |
      auto maybe_letter = tiles.CharToNumber(rows[src_i][src_j]);
      if (maybe_letter) {
        rows_[i][j] = maybe_letter.value();
        is_empty_ = false;
      } else {
        rows_[i][j] = 0;
      }
    }
  }
}
