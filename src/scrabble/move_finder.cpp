#include "src/scrabble/move_finder.h"

#include <range/v3/all.hpp>

#include "absl/container/flat_hash_set.h"
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
    // LOG(INFO) << "blank_letter: " <<
    // tiles_.NumberToChar(blank_letter).value();
    std::vector<int> positions;
    for (int i = 0; i < word.size(); ++i) {
      if (word[i] == blank_letter) {
        positions.push_back(i);
      }
    }
    // for (int pos : positions) {
    //   LOG(INFO) << "pos: " << pos;
    // }
    if (word_counts[blank_letter] == rack_counts[blank_letter] + 1) {
      for (int i = 0; i < positions.size(); ++i) {
        LetterString ret_word = word;
        // LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
        ret_word[positions[i]] += tiles_.BlankIndex();
        // LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
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

int MoveFinder::WordMultiplier(const Board& board, Move::Dir direction,
                               int start_row, int start_col,
                               int num_tiles) const {
  int ret = 1;
  int row = start_row;
  int col = start_col;
  int tiles_used = 0;
  while (row < 15 && col < 15) {
    Letter sq = board.At(row, col);
    if (!sq) {
      ret *= board_layout_.WordMultiplier(row, col);
      tiles_used++;
      if (tiles_used >= num_tiles) {
        return ret;
      }
    }
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  return ret;
}

int MoveFinder::HookSum(const Board& board, Move::Dir direction, int start_row,
                        int start_col, int num_tiles) const {
  int ret = 0;
  int row = start_row;
  int col = start_col;
  int tiles_used = 0;
  while (row < 15 && col < 15) {
    Letter sq = board.At(row, col);
    if (!sq) {
      const auto cross = CrossAt(board, direction, row, col, false);
      if (cross.has_value()) {
        const int score = tiles_.Score(cross.value());
        ret += score * board_layout_.WordMultiplier(row, col);
      }
      tiles_used++;
      if (tiles_used >= num_tiles) {
        return ret;
      }
    }
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  return ret;
}

int MoveFinder::ThroughScore(const Board& board, Move::Dir direction,
                             int start_row, int start_col,
                             int num_tiles) const {
  int ret = 0;
  int row = start_row;
  int col = start_col;
  int num_rack_tiles = 0;
  while (row < 15 && col < 15) {
    Letter letter = board.At(row, col);
    if (letter) {
      if (letter < tiles_.BlankIndex()) {
        ret += tiles_.Score(letter);
      }
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

int MoveFinder::WordScore(const Board& board, const Move& move,
                          int word_multiplier) const {
  int word = 0;
  int crossing = 0;
  int row = move.StartRow();
  int col = move.StartCol();
  for (Letter letter : move.Letters()) {
    if (letter) {
      if (letter < tiles_.BlankIndex()) {
        const int tile_score = tiles_.Score(letter);
        const int letter_multiplier = board_layout_.LetterMultiplier(row, col);
        word += tile_score * letter_multiplier;

        const auto cross = CrossAt(board, move.Direction(), row, col, false);
        if (cross.has_value()) {
          crossing += tile_score * letter_multiplier *
                      board_layout_.WordMultiplier(row, col);
        }
      }
    }
    if (move.Direction() == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  return word * word_multiplier + crossing;
}

std::vector<Move> MoveFinder::FindWords(const Rack& rack, const Board& board,
                                        Move::Dir direction, int start_row,
                                        int start_col, int num_tiles) const {
  std::vector<Move> moves;
  const auto subsets = rack.Subsets(tiles_);
  const absl::uint128 through_product =
      AbsorbThroughTiles(board, direction, start_row, start_col, num_tiles);
  const int word_multiplier =
      WordMultiplier(board, direction, start_row, start_col, num_tiles);
  const int hook_sum =
      HookSum(board, direction, start_row, start_col, num_tiles);
  const int through_score =
      ThroughScore(board, direction, start_row, start_col, num_tiles) *
      word_multiplier;
  const int bonus = num_tiles == 7 ? 50 : 0;
  for (int num_blanks = 0; num_blanks <= rack.NumBlanks(tiles_); ++num_blanks) {
    // LOG(INFO) << "num_blanks: " << num_blanks;
    for (const auto& subset : subsets) {
      const absl::uint128& product = subset.first;
      // LOG(INFO) << "product: " << product;
      auto letters = subset.second;
      // LOG(INFO) << "letters: " << tiles_.ToString(letters).value();
      if (letters.size() + num_blanks != num_tiles) {
        continue;
      }
      const auto words =
          anagram_map_.Words(product * through_product, num_blanks);
      auto span_join = words.Spans() | ranges::view::join;
      for (const auto& word : span_join) {
        // LOG(INFO) << "word: " << tiles_.ToString(word).value();
        auto played_tiles = ZeroPlayedThroughTiles(board, direction, start_row,
                                                   start_col, word);
        if (!played_tiles) {
          // LOG(INFO) << "  does not fit on board here";
          continue;
        }
        // LOG(INFO) << "played_tiles: " <<
        // tiles_.ToString(*played_tiles).value();
        if (num_blanks == 0) {
          Move move(direction, start_row, start_col, *played_tiles);
          if (CheckHooks(board, move)) {
            const int word_score = WordScore(board, move, word_multiplier);
            const int score = word_score + through_score + hook_sum + bonus;
            move.SetScore(score);
            moves.push_back(move);
          }
        } else {
          const auto blankified = Blankify(letters, *played_tiles);
          for (const auto& blank_word : blankified) {
            Move move(direction, start_row, start_col, blank_word);
            if (CheckHooks(board, move)) {
              const int word_score = WordScore(board, move, word_multiplier);
              const int score = word_score + through_score + hook_sum + bonus;
              move.SetScore(score);
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
                                                 int square_row, int square_col,
                                                 bool unblank) const {
  // LOG(INFO) << "CrossAt(" << square_row << ", " << square_col << ")";
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
    // LOG(INFO) << "  row: " << row << ", col: " << col;
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
  // LOG(INFO) << "  spots_before: " << spots_before;
  row = square_row;
  col = square_col;
  if (play_dir == Move::Across) {
    row -= spots_before;
  } else {
    col -= spots_before;
  }

  for (int i = 0; i < spots_before; ++i) {
    // LOG(INFO) << "  row: " << row << ", col: " << col;
    Letter letter = board.At(row, col);
    // LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    if (letter > tiles_.BlankIndex()) {
      if (unblank) {
        // Turn blanked letter into regular letter to check words.
        letter -= tiles_.BlankIndex();
      } else {
        // Turn blanked letter into blank to calculate scores.
        letter = tiles_.BlankIndex();
      }
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
    // LOG(INFO) << "  row: " << row << ", col: " << col;
    // LOG(INFO) << "  letter: " << tiles_.NumberToChar(letter).value();
    if (letter) {
      if (letter > tiles_.BlankIndex()) {
        if (unblank) {
          // Turn blanked letter into regular letter to check words.
          letter -= tiles_.BlankIndex();
        } else {
          // Turn blanked letter into blank to calculate scores.
          letter = tiles_.BlankIndex();
        }
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
    // LOG(INFO) << "letter: " << tiles_.NumberToChar(letter).value()
    //           << " row: " << row << " col: " << col;
    if (letter) {
      if (letter > tiles_.BlankIndex()) {
        letter -= tiles_.BlankIndex();
      }
      const auto cross = CrossAt(board, move.Direction(), row, col, true);
      if (cross) {
        const uint32_t hooks = anagram_map_.Hooks(*cross);
        std::string hooks_str;
        for (int i = 1; i < tiles_.BlankIndex(); ++i) {
          if (hooks & (1 << i)) {
            hooks_str += tiles_.NumberToChar(i).value();
          }
        }
        // LOG(INFO) << "hooks: " << hooks_str;
        if ((hooks & (1 << letter)) == 0) {
          // LOG(INFO) << "letter " << tiles_.NumberToChar(letter).value()
          //           << " does not hook";
          return false;
        }
      } else {
        // LOG(INFO) << "cross was unconstrained";
      }
    } else {
      // LOG(INFO) << "letter was empty";
    }
    if (move.Direction() == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  // LOG(INFO) << "all hooks are ok";
  return true;
}

void MoveFinder::FindSpots(int rack_tiles, const Board& board,
                           Move::Dir direction,
                           std::vector<MoveFinder::Spot>* spots) const {
  for (int start_row = 0; start_row < 15; start_row++) {
    for (int start_col = 0; start_col < 15; start_col++) {
      // LOG(INFO) << "Starting at " << start_row << ", " << start_col;
      bool crossing = false;
      bool through = false;
      int previous_sq_row = start_row;
      int previous_sq_col = start_col;
      if (direction == Move::Across) {
        previous_sq_col--;
      } else {
        previous_sq_row--;
      }
      if (previous_sq_row >= 0 && previous_sq_col >= 0) {
        if (board.At(previous_sq_row, previous_sq_col)) {
          // LOG(INFO) << "previous square was occupied";
          //  If the previous square is occupied, this isn't a valid starting
          //  square. All moves covering this square would be covered by the
          //  spot starting at the beginning of the previous word.
          continue;
        }
      }
      int sq_row = start_row;
      int sq_col = start_col;
      int num_tiles = 0;
      absl::flat_hash_set<int> recorded_num_tiles;
      while (sq_row < 15 && sq_col < 15) {
        const Letter sq = board.At(sq_row, sq_col);
        // LOG(INFO) << "  looking at " << sq_row << ", " << sq_col << " ("
        //           << tiles_.NumberToChar(sq).value() << ")";
        if (sq) {
          // LOG(INFO) << "let through = true";
          through = true;
        } else {
          num_tiles++;
          if (num_tiles > rack_tiles) {
            break;
          }
          const auto cross = CrossAt(board, direction, sq_row, sq_col, true);
          if (cross.has_value()) {
            // LOG(INFO) << "cross: " << tiles_.ToString(*cross).value()
            //           << ", crossing = true";
            crossing = true;
          }
        }

        // LOG(INFO) << "num_tiles: " << num_tiles;
        if ((through || crossing) && recorded_num_tiles.count(num_tiles) == 0) {
          absl::optional<LetterString> across_cross = absl::nullopt;
          absl::optional<LetterString> down_cross = absl::nullopt;
          if (!sq) {
            across_cross = CrossAt(board, Move::Across, sq_row, sq_col, true);
            down_cross = CrossAt(board, Move::Down, sq_row, sq_col, true);
          }
          if ((num_tiles == 1) && across_cross.has_value() &&
              down_cross.has_value()) {
            if (direction == Move::Across) {
              // LOG(INFO) << "across_cross: "
              //           << tiles_.ToString(*across_cross).value();
              // LOG(INFO) << "down_cross: "
              //           << tiles_.ToString(*down_cross).value();
              recorded_num_tiles.insert(num_tiles);
              int tiles_before_first_played_tile = 0;
              if (down_cross->length() >= across_cross->length()) {
                for (Letter letter : down_cross.value()) {
                  if (letter) {
                    tiles_before_first_played_tile++;
                  } else {
                    break;
                  }
                }
                // LOG(INFO) << "pushing one tile play at " << sq_row << ", "
                //           << sq_col << " as 'across'";
                // LOG(INFO) << "start_row: " << sq_row << ", start_col: "
                //           << sq_col - tiles_before_first_played_tile
                //           << ", num_tiles: " << num_tiles
                //           << ", crossing: " << crossing
                //           << ", through: " << through;
                spots->push_back({Move::Across, sq_row,
                                  sq_col - tiles_before_first_played_tile,
                                  num_tiles});
              } else {
                for (Letter letter : across_cross.value()) {
                  if (letter) {
                    tiles_before_first_played_tile++;
                  } else {
                    break;
                  }
                }
                // LOG(INFO) << "pushing one tile play at " << sq_row << ", "
                //           << sq_col << " as 'down'";
                // LOG(INFO) << "start_row: "
                //           << sq_row - tiles_before_first_played_tile
                //           << ", start_col: " << sq_col
                //           << ", num_tiles: " << num_tiles
                //           << ", crossing: " << crossing
                //           << ", through: " << through;
                spots->push_back({Move::Down,
                                  sq_row - tiles_before_first_played_tile,
                                  sq_col, num_tiles});
              }
            }
          } else if (num_tiles > 1 ||
                     ((num_tiles == 1) && through && !crossing)) {
            recorded_num_tiles.insert(num_tiles);
            // LOG(INFO) << "start_row: " << start_row
            //           << ", start_col: " << start_col
            //           << ", num_tiles: " << num_tiles
            //           << ", crossing: " << crossing << ", through: " <<
            //           through;
            spots->push_back({direction, start_row, start_col, num_tiles});
          }
        }

        if (direction == Move::Across) {
          sq_col++;
        } else {
          sq_row++;
        }
      }
    }
  }
}

std::vector<MoveFinder::Spot> MoveFinder::FindSpots(const Rack& rack,
                                                    const Board& board) const {
  std::vector<MoveFinder::Spot> spots;
  if (board.IsEmpty()) {
    for (int num_tiles = 2; num_tiles <= rack.NumTiles(); num_tiles++) {
      for (int start_col = 7 + 1 - num_tiles; start_col <= 7; start_col++) {
        spots.push_back({Move::Across, 7, start_col, num_tiles});
      }
    }
  } else {
    FindSpots(rack.NumTiles(), board, Move::Across, &spots);
    FindSpots(rack.NumTiles(), board, Move::Down, &spots);
  }
  return spots;
}

std::vector<Move> MoveFinder::FindMoves(const Rack& rack,
                                        const Board& board) const {
  std::vector<Move> moves;
  const std::vector<MoveFinder::Spot> spots = FindSpots(rack, board);
  for (const MoveFinder::Spot& spot : spots) {
    const auto words = FindWords(rack, board, spot.Direction(), spot.StartRow(),
                                 spot.StartCol(), spot.NumTiles());
    moves.insert(moves.end(), words.begin(), words.end());
  }
  return moves;
}
