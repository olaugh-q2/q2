#include "src/scrabble/move_finder.h"

#include <range/v3/all.hpp>

#include "glog/logging.h"

std::vector<Move> MoveFinder::FindExchanges(const Rack& rack) const {
  std::vector<Move> moves;
  const auto subsets = rack.Subsets(tiles_);
  for (int num_blanks = 0; num_blanks <= rack.NumBlanks(tiles_); ++num_blanks) {
    for (const auto& subset : subsets) {
      // const absl::uint128& product = subset.first;
      LetterString letters = subset.second;
      for (int i = 0; i < num_blanks; ++i) {
        letters.push_back(tiles_.BlankIndex());
      }
      moves.push_back(Move(letters));
    }
  }
  return moves;
}

absl::optional<LetterString> MoveFinder::ZeroPlayedThroughTiles(
    const Board& board, Move::Dir direction, int start_row, int start_col,
    const LetterString& word) const {
  LetterString ret = word;
  int row = start_row;
  int col = start_col;
  for (int i = 0; i < word.size(); ++i) {
    const Letter word_letter = word[i];
    const Letter board_letter = board.At(row, col);
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
    if (!board_letter) {
      continue;
    }
    if (board_letter == word_letter) {
      ret[i] = 0;
    } else {
      return absl::nullopt;
    }
  }
  return ret;
}

std::vector<LetterString> MoveFinder::Blankify(const LetterString& rack_letters,
                                               const LetterString& word) const {
  std::array<int, 32> rack_counts = rack_letters.Counts();
  std::array<int, 32> word_counts = word.Counts();
  std::vector<LetterString> ret;
  std::vector<Letter> blank_letters;
  for (Letter i = 1; i < tiles_.BlankIndex(); ++i) {
    if (word_counts[i] > rack_counts[i]) {
      blank_letters.push_back(i);
    }
  }
  CHECK_GE(blank_letters.size(), 1);
  CHECK_LE(blank_letters.size(), 2);
  if (blank_letters.size() == 1) {
    const Letter blank_letter = blank_letters[0];
    LOG(INFO) << "blank_letter: " << tiles_.NumberToChar(blank_letter).value();
    std::vector<int> positions;
    for (int i = 0; i < word.size(); ++i) {
      if (word[i] == blank_letter) {
        positions.push_back(i);
      }
    }
    for (int pos : positions) {
      LOG(INFO) << "pos: " << pos;
    }
    if (word_counts[blank_letter] == rack_counts[blank_letter] + 1) {
      for (int i = 0; i < positions.size(); ++i) {
        LetterString ret_word = word;
        LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
        ret_word[positions[i]] += tiles_.BlankIndex();
        LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
        ret.push_back(ret_word);
      }
    } else {
      CHECK_EQ(word_counts[blank_letter], rack_counts[blank_letter] + 2);
      for (int i = 0; i < positions.size() - 1; ++i) {
        for (int j = i + 1; j < positions.size(); ++j) {
          LetterString ret_word = word;
          ret_word[positions[i]] += tiles_.BlankIndex();
          ret_word[positions[j]] += tiles_.BlankIndex();
          ret.push_back(ret_word);
        }
      }
    }
  } else {
    CHECK_EQ(blank_letters.size(), 2);
    const Letter blank_letter1 = blank_letters[0];
    const Letter blank_letter2 = blank_letters[1];
    std::vector<int> positions1;
    std::vector<int> positions2;
    for (int i = 0; i < word.size(); ++i) {
      if (word[i] == blank_letter1) {
        positions1.push_back(i);
      } else if (word[i] == blank_letter2) {
        positions2.push_back(i);
      }
    }
    for (int i = 0; i < positions1.size(); ++i) {
      LetterString word1 = word;
      word1[positions1[i]] += tiles_.BlankIndex();
      for (int j = 0; j < positions2.size(); ++j) {
        LetterString ret_word = word1;
        ret_word[positions2[j]] += tiles_.BlankIndex();
        ret.push_back(ret_word);
      }
    }
  }
  return ret;
}

absl::uint128 MoveFinder::AbsorbThroughTiles(const Board& board,
                                             Move::Dir direction, int start_row,
                                             int start_col,
                                             int num_tiles) const {
  absl::uint128 ret = 1;
  int row = start_row;
  int col = start_col;
  int num_rack_tiles = 0;
  while (row < 15 && col < 15) {
    Letter letter = board.At(row, col);
    if (letter > tiles_.BlankIndex()) {
      letter -= tiles_.BlankIndex();
    }
    if (letter) {
      ret *= tiles_.Prime(letter);
    } else {
      if (num_rack_tiles == num_tiles) {
        return ret;
      }
      num_rack_tiles++;
    }
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  return ret;
}

std::vector<Move> MoveFinder::FindWords(const Rack& rack, const Board& board,
                                        Move::Dir direction, int start_row,
                                        int start_col, int num_tiles) const {
  std::vector<Move> moves;
  const auto subsets = rack.Subsets(tiles_);
  const absl::uint128 through_product =
      AbsorbThroughTiles(board, direction, start_row, start_col, num_tiles);
  for (int num_blanks = 0; num_blanks <= rack.NumBlanks(tiles_); ++num_blanks) {
    LOG(INFO) << "num_blanks: " << num_blanks;
    for (const auto& subset : subsets) {
      const absl::uint128& product = subset.first;
      // LOG(INFO) << "product: " << product;
      auto letters = subset.second;
      LOG(INFO) << "letters: " << tiles_.ToString(letters).value();
      if (letters.size() + num_blanks != num_tiles) {
        continue;
      }
      const auto words =
          anagram_map_.Words(product * through_product, num_blanks);
      auto span_join = words.Spans() | ranges::view::join;
      for (const auto& word : span_join) {
        LOG(INFO) << "word: " << tiles_.ToString(word).value();
        auto played_tiles = ZeroPlayedThroughTiles(board, direction, start_row,
                                                   start_col, word);
        if (!played_tiles) {
          LOG(INFO) << "  does not fit on board here";
          continue;
        }
        LOG(INFO) << "played_tiles: " << tiles_.ToString(*played_tiles).value();
        if (num_blanks == 0) {
          const Move move(direction, start_row, start_col, *played_tiles);
          if (CheckHooks(board, move)) {
            moves.push_back(move);
          }
        } else {
          const auto blankified = Blankify(letters, *played_tiles);
          for (const auto& blank_word : blankified) {
            const Move move(direction, start_row, start_col, blank_word);
            if (CheckHooks(board, move)) {
              moves.push_back(move);
            }
          }
        }
      }
    }
  }
  return moves;
}

absl::optional<LetterString> MoveFinder::CrossAt(const Board& board,
                                                 Move::Dir play_dir,
                                                 int square_row,
                                                 int square_col) const {
  LOG(INFO) << "CrossAt(" << square_row << ", " << square_col << ")";
  CHECK_EQ(board.At(square_row, square_col), 0);
  LetterString ret;
  int row = square_row;
  int col = square_col;
  if (play_dir == Move::Across) {
    row--;
  } else {
    col--;
  }
  int spots_before = 0;
  while (row >= 0 && col >= 0) {
   LOG(INFO) << "  row: " << row << ", col: " << col;
    Letter letter = board.At(row, col);
    if (letter) {
      spots_before++;
    } else {
      break;
    }
    if (play_dir == Move::Across) {
      row--;
    } else {
      col--;
    }
  }
  LOG(INFO) << "  spots_before: " << spots_before;
  row = square_row;
  row = square_col;
  if (play_dir == Move::Across) {
    row -= spots_before;
  } else {
    col -= spots_before;
  }

  for (int i = 0; i < spots_before; ++i) {
    LOG(INFO) << "  row: " << row << ", col: " << col;
    Letter letter = board.At(row, col);
    LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    if (letter > tiles_.BlankIndex()) {
      letter -= tiles_.BlankIndex();
    }
    ret.push_back(letter);
    if (play_dir == Move::Across) {
      row++;
    } else {
      col++;
    }
  }
  ret.push_back(0);
  row = square_row;
  col = square_col;
  if (play_dir == Move::Across) {
    row++;
  } else {
    col++;
  }
  while (row < 15 && col < 15) {
    Letter letter = board.At(row, col);
    LOG(INFO) << "  row: " << row << ", col: " << col;
    LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    if (letter) {
      if (letter > tiles_.BlankIndex()) {
        letter -= tiles_.BlankIndex();
      }
      ret.push_back(letter);
    } else {
      break;
    }
    if (play_dir == Move::Across) {
      row++;
    } else {
      col++;
    }
  }
  if (ret.length() > 1) {
    return ret;
  } else {
    return absl::nullopt;
  }
}

bool MoveFinder::CheckHooks(const Board& board, const Move& move) const {
  int row = move.StartRow();
  int col = move.StartCol();
  for (Letter letter : move.Letters()) {
    //LOG(INFO) << "letter: " << tiles_.NumberToChar(letter).value()
    //          << " row: " << row << " col: " << col;
    if (letter) {
      if (letter > tiles_.BlankIndex()) {
        letter -= tiles_.BlankIndex();
      }
      const auto cross = CrossAt(board, move.Direction(), row, col);
      if (cross) {
        const uint32_t hooks = anagram_map_.Hooks(*cross);
        std::string hooks_str;
        for (int i = 1; i < tiles_.BlankIndex(); ++i) {
          if (hooks & (1 << i)) {
            hooks_str += tiles_.NumberToChar(i).value();
          }
        }
        LOG(INFO) << "hooks: " << hooks_str;
        if ((hooks & (1 << letter)) == 0) {
          LOG(INFO) << "letter " << tiles_.NumberToChar(letter).value()
                    << " does not hook";
          return false;
        }
      } else {
        LOG(INFO) << "cross was unconstrained";
      }
    } else {
      LOG(INFO) << "letter was empty";
    }
    if (move.Direction() == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  //LOG(INFO) << "all hooks are ok";
  return true;
}