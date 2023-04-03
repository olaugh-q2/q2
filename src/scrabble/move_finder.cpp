#include "src/scrabble/move_finder.h"

#include <range/v3/all.hpp>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_join.h"
#include "glog/logging.h"

std::vector<Move> MoveFinder::FindExchanges(const Rack& rack) const {
  std::vector<Move> moves;
  const auto subsets = rack.Subsets(tiles_);
  const auto counts = rack.Counts();
  for (int num_blanks = 0; num_blanks <= rack.NumBlanks(tiles_); ++num_blanks) {
    for (const auto& subset : subsets) {
      LetterString letters = subset.second;
      auto leave_counts = counts;
      for (const auto& letter : letters) {
        leave_counts[letter]--;
      }
      for (int i = 0; i < num_blanks; ++i) {
        letters.push_back(tiles_.BlankIndex());
        leave_counts[tiles_.BlankIndex()]--;
      }
      if (letters.size() == 0) {
        // Pass 0 is a special case and we don't want to handle it as an
        // exchange because we don't have leave values for it and generally it
        // should be penalized for giving away too much information. Unless
        // we're being very clever, it should be a last resort.
        continue;
      }
      uint64_t leave_product = 1;
      LetterString leave;
      for (Letter letter = tiles_.FirstLetter(); letter <= tiles_.BlankIndex();
           ++letter) {
        for (int j = 0; j < leave_counts[letter]; ++j) {
          leave.push_back(letter);
          leave_product *= tiles_.Prime(letter);
        }
      }

      Move move(letters);
      const float leave_value = leaves_.Value(leave_product);
      move.SetLeave(leave);
      move.SetLeaveValue(leave_value);
      move.ComputeEquity();
      moves.push_back(move);
    }
  }
  return moves;
}

bool MoveFinder::FitsWithPlayedThroughTiles(const Board& board,
                                            Move::Dir direction, int start_row,
                                            int start_col,
                                            const LetterString& word) const {
  int row = start_row;
  int col = start_col;
  for (const Letter& word_letter : word) {
    Letter board_letter = board.At(row, col);
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
    if (!board_letter) {
      continue;
    }
    if (board_letter > tiles_.BlankIndex()) {
      board_letter -= tiles_.BlankIndex();
    }
    if (board_letter != word_letter) {
      return false;
    }
  }
  return true;
}

LetterString MoveFinder::ZeroPlayedThroughTiles(
    const Board& board, Move::Dir direction, int start_row, int start_col,
    const LetterString& word) const {
  LetterString ret = word;
  int row = start_row;
  int col = start_col;
  for (int i = 0; i < ret.size(); ++i) {
    Letter board_letter = board.At(row, col);
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
    if (board_letter) {
      ret[i] = 0;
    }
  }
  return ret;
}

std::vector<LetterString> MoveFinder::Blankify(const LetterString& rack_letters,
                                               const LetterString& word) const {
  // LOG(INFO) << "Blankify('" << tiles_.ToString(rack_letters).value() << "',
  // '"
  //           << tiles_.ToString(word).value() << "')";
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
  // LOG(INFO) << "blank_letters.size(): " << blank_letters.size();
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
          // LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
          ret_word[positions[i]] += tiles_.BlankIndex();
          ret_word[positions[j]] += tiles_.BlankIndex();
          // LOG(INFO) << "ret_word: " << tiles_.ToString(ret_word).value();
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
      ret += score_table_[direction - 1][row][col];
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
                          int word_multiplier) {
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

        const auto cross = hook_table_[move.Direction() - 1][row][col];
        if (cross != kNotTouching) {
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

const std::vector<std::pair<absl::uint128, LetterString>>& MoveFinder::Subracks(
    const Rack& rack, int num_tiles) {
  if (subracks_.empty()) {
    // LOG(INFO) << "generating subracks for " <<
    // tiles_.ToString(rack.Letters()).value();
    auto subsets = rack.Subsets(tiles_);
    subracks_.reserve(subsets.size());
    for (const auto& subset : subsets) {
      subracks_.push_back(subset);
    }
  }
  return subracks_;
}

void MoveFinder::CacheRackPartitions(const Rack& rack) {
  // LOG(INFO) << "CacheRackPartitions(...)";
  const int rack_blanks = rack.NumBlanks(tiles_);
  auto subsets = rack.Subsets(tiles_);
  // LOG(INFO) << "Got subsets";
  const auto rack_counts = rack.Counts();
  for (int i = 0; i <= 7; i++) {
    rack_partitions_[i].clear();
    best_leave_at_size_[i] = -999999;
    rack_word_of_length_[i] = false;
  }
  for (int blanks = 0; blanks <= rack_blanks; ++blanks) {
    for (const auto& subset : subsets) {
      const uint64_t used_product = static_cast<uint64_t>(subset.first);
      LetterString used_letters = subset.second;
      for (int i = 0; i < blanks; ++i) {
        used_letters.push_back(tiles_.BlankIndex());
      }
      const int size = used_letters.size();
      /*
      const auto words = anagram_map_.Words(used_product, blanks);
      auto span_join = words.Spans() | ranges::view::join;
      const bool has_word = span_join.begin() != span_join.end();
      */
      bool has_word = true;  // hack to turn off
      if (used_letters.size() >= 6) {
        has_word = anagram_map_.HasWord(used_product, blanks);
      }
      if (has_word) {
        rack_word_of_length_[size] = true;
      }

      auto leave_counts = rack_counts;
      for (Letter letter : used_letters) {
        leave_counts[letter]--;
      }
      uint64_t leave_product = 1;
      LetterString leave;
      for (Letter letter = tiles_.FirstLetter(); letter <= tiles_.BlankIndex();
           ++letter) {
        for (int i = 0; i < leave_counts[letter]; ++i) {
          leave.push_back(letter);
          leave_product *= tiles_.Prime(letter);
        }
      }
      const int leave_size = leave.size();
      const float leave_value =
          (leave_size > 6) ? -999999 : leaves_.Value(leave_product);
      const RackPartition rp(used_product, blanks, used_letters, leave,
                             leave_value);
      rack_partitions_[size].push_back(rp);
      if (leave_value > best_leave_at_size_[leave_size]) {
        best_leave_at_size_[leave_size] = leave_value;
      }
    }
  }
}

std::vector<Move> MoveFinder::FindWords(const Rack& rack, const Board& board,
                                        const Spot& spot,
                                        MoveFinder::RecordMode record_mode,
                                        float best_equity) {
  const auto direction = spot.Direction();
  const int start_row = spot.StartRow();
  const int start_col = spot.StartCol();
  const int num_tiles = spot.NumTiles();
  // LOG(INFO) << "FindWords(" << direction << ", " << start_row << ", "
  //           << start_col << ", " << num_tiles << ")"
  //           << " max equity: " << spot.MaxEquity();
  std::vector<Move> moves;
  const absl::uint128 through_product =
      AbsorbThroughTiles(board, direction, start_row, start_col, num_tiles);
  int partitions_used = 0;
  auto& partitions = rack_partitions_[num_tiles];
  for (auto& partition : partitions) {
    if ((through_product == 1) && !HasWord(&partition)) {
      // LOG(INFO) << "skipping partition without word: "
      //           << tiles_.ToString(partition.UsedLetters()).value();
      continue;
    }
    const auto num_blanks = partition.NumBlanks();
    // LOG(INFO) << "num_blanks: " << num_blanks;
    const absl::uint128 product = partition.UsedProduct();
    const auto& letters = partition.UsedLetters();
    const auto& leave = partition.LeftLetters();
    const auto leave_value = partition.LeaveValue();
    // LOG(INFO) << "partition of " << tiles_.ToString(letters).value() << " / "
    //          << tiles_.ToString(leave).value() << " (" << leave_value << ")";
    if (record_mode == MoveFinder::RecordBest) {
      // LOG(INFO) << "best_equity: " << best_equity
      //           << ", leave_value: " << leave_value
      //           << ", spot.MaxScore: " << spot.MaxScore();
      if (leave_value + spot.MaxScore() < best_equity + 1e-5) {
        // LOG(INFO) << "...skipping partition";
        continue;
      }
      /*
      const int max_word_score = SpotMaxScore(rack.Letters(), board, spot);
      const float max_equity = spot.ExtraScore() + max_word_score + leave_value;
      // LOG(INFO) << "max_word_score: " << max_word_score
      //           << ", max_equity: " << max_equity
      //           << ", best_equity: " << best_equity;
      if (max_equity < best_equity + 1e-5) {
        continue;
      }
      */
    }
    partitions_used++;
    const auto words =
        (through_product == 1)
            ? anagram_map_.Words(partition.WordIterator(), product)
            : anagram_map_.Words(product * through_product, num_blanks);
    auto span_join = words.Spans() | ranges::view::join;
    for (const auto& word : span_join) {
      // LOG(INFO) << "word: " << tiles_.ToString(word).value();
      if (!FitsWithPlayedThroughTiles(board, direction, start_row, start_col,
                                      word)) {
        // LOG(INFO) << "  does not fit with played through tiles";
        continue;
      }
      const LetterString played_tiles =
          ZeroPlayedThroughTiles(board, direction, start_row, start_col, word);
      // LOG(INFO) << "played_tiles (zeroed): "
      //           << tiles_.ToString(played_tiles).value();

      // Score prospective move before checking its hooks or blankifying it.
      // If it can't possibly be best, continue.
      Move move(direction, start_row, start_col, played_tiles);
      const int word_score = WordScore(board, move, spot.WordMultiplier());
      // LOG(INFO) << "scored";
      const int score = spot.ExtraScore() + word_score;
      // const int score = word_score + through_score + hook_sum + bonus;
      // CHECK_EQ(score, spot.ExtraScore() + word_score);
      move.SetScore(score);
      move.SetLeave(leave);
      move.SetLeaveValue(leave_value);
      move.ComputeEquity();
      if (record_mode == RecordMode::RecordBest &&
          move.Equity() < best_equity + 1e-5) {
        continue;
      }

      if (CheckHooks(board, move)) {
        if (num_blanks == 0) {
          // std::stringstream ss;
          // move.Display(tiles_, ss);
          // LOG(INFO) << " found move " << ss.str();
          //  CHECK_GE(spot.MaxEquity(), move.Equity()) << ss.str();
          CHECK_LE(move.Equity(), leave_value + spot.MaxScore() + 1e-5);
          if (record_mode == RecordMode::RecordAll ||
              (record_mode == RecordMode::RecordBest &&
               move.Equity() > best_equity + 1e-5)) {
            best_equity = move.Equity();
            if ((record_mode == RecordMode::RecordBest) &&
                move.Equity() + 1e-5 > spot.MaxEquity()) {
              // LOG(INFO) << "Returning best move possible from this spot";
              return {move};
            }
            moves.push_back(move);
          }
        } else {
          const auto blankified = Blankify(letters, played_tiles);
          // LOG(INFO) << "blankfied";
          for (const auto& blank_word : blankified) {
            Move blank_move(direction, start_row, start_col, blank_word);
            const int word_score =
                WordScore(board, blank_move, spot.WordMultiplier());
            const int score = spot.ExtraScore() + word_score;
            // const int score = word_score + through_score + hook_sum +
            // bonus; CHECK_EQ(score, spot.ExtraScore() + word_score);
            blank_move.SetScore(score);
            blank_move.SetLeave(leave);
            blank_move.SetLeaveValue(leave_value);
            blank_move.ComputeEquity();
            CHECK_LE(blank_move.Equity(), leave_value + spot.MaxScore());

            // std::stringstream ss;
            // blank_move.Display(tiles_, ss);
            // LOG(INFO) << " found move " << ss.str();
            //  CHECK_GE(spot.MaxEquity(), blank_move.Equity())
            //      << "move " << ss.str();
            if (record_mode == RecordMode::RecordAll ||
                (record_mode == RecordMode::RecordBest &&
                 blank_move.Equity() > best_equity + 1e-5)) {
              best_equity = blank_move.Equity();
              if ((record_mode == RecordMode::RecordBest) &&
                  blank_move.Equity() + 1e-5 > spot.MaxEquity()) {
                // LOG(INFO) << "Returning best move possible from this spot";
                return {blank_move};
              }
              moves.push_back(blank_move);
            }
          }
        }
      } else {
        // LOG(INFO) << "  doesn't hook.";
      }
    }
  }
  // LOG(INFO) << "  partitions searched: " << partitions_used << " out of "
  //           << partitions.size();
  return moves;
}

void MoveFinder::CacheCrossesAndScores(const Board& board) {
  // LOG(INFO) << "CacheCrossesAndScores(...)";
  for (int row = 0; row < 15; ++row) {
    for (int col = 0; col < 15; ++col) {
      Letter letter = board.At(row, col);
      if (letter) {
        continue;
      }
      const int word_multiplier = board_layout_.WordMultiplier(row, col);
      auto across = CrossAt(board, Move::Dir::Across, row, col);
      auto down = CrossAt(board, Move::Dir::Down, row, col);
      if (across.has_value()) {
        // LOG(INFO) << "across: " << tiles_.ToString(across.value()).value();
        int score_sum = 0;
        for (auto& letter : across.value()) {
          if (letter < tiles_.BlankIndex()) {
            score_sum += tiles_.Score(letter);
          } else {
            letter -= tiles_.BlankIndex();
          }
        }
        // LOG(INFO) << "across: " << tiles_.ToString(across.value()).value();
        // LOG(INFO) << "score_sum: " << score_sum;
        score_table_[0][row][col] = score_sum * word_multiplier;
        hook_table_[0][row][col] = anagram_map_.Hooks(across.value());
      } else {
        hook_table_[0][row][col] = kNotTouching;
        score_table_[0][row][col] = 0;
      }
      if (down.has_value()) {
        // LOG(INFO) << "down: " << tiles_.ToString(down.value()).value();
        int score_sum = 0;
        for (auto& letter : down.value()) {
          if (letter < tiles_.BlankIndex()) {
            score_sum += tiles_.Score(letter);
          } else {
            letter -= tiles_.BlankIndex();
          }
        }
        // LOG(INFO) << "down: " << tiles_.ToString(down.value()).value();
        // LOG(INFO) << "score_sum: " << score_sum;
        score_table_[1][row][col] = score_sum * word_multiplier;
        hook_table_[1][row][col] = anagram_map_.Hooks(down.value());
      } else {
        hook_table_[1][row][col] = kNotTouching;
        score_table_[1][row][col] = 0;
      }
    }
  }
}

absl::optional<LetterString> MoveFinder::CrossAt(const Board& board,
                                                 Move::Dir play_dir,
                                                 int square_row,
                                                 int square_col) const {
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

bool MoveFinder::CheckHooks(const Board& board, const Move& move) {
  int row = move.StartRow();
  int col = move.StartCol();
  for (Letter letter : move.Letters()) {
    // LOG(INFO) << "letter: " << tiles_.NumberToChar(letter).value()
    //           << " row: " << row << " col: " << col;
    if (letter) {
      if (letter > tiles_.BlankIndex()) {
        letter -= tiles_.BlankIndex();
      }
      const auto cross = hook_table_[move.Direction() - 1][row][col];
      if ((cross & (1 << letter)) == 0) {
        // LOG(INFO) << "letter " << tiles_.NumberToChar(letter).value()
        //           << " does not hook";
        return false;
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
                           std::vector<MoveFinder::Spot>* spots) {
  // std::vector<std::string> dir_names = {"", "Across", "Down"};
  // LOG(INFO) << "FindSpots(...) direction: " << dir_names[direction];
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
          // LOG(INFO) << "checking cross at " << sq_row << ", " << sq_col;
          const auto cross = hook_table_[direction - 1][sq_row][sq_col];
          if (cross != kNotTouching) {
            // LOG(INFO) << "cross: " << cross << ", crossing = true";
            if (cross == 0) {
              // LOG(INFO) << "Unhookable square interrupts the spots";
              //  Unhookable square interrupts the spots starting here.
              break;
            }
            crossing = true;
          }
        }

        bool could_have_word = through | rack_word_of_length_[num_tiles] |
                               ((num_tiles == 1) && crossing);
        // LOG(INFO) << "num_tiles: " << num_tiles;
        if (could_have_word && (through || crossing) &&
            recorded_num_tiles.count(num_tiles) == 0) {
          auto across_hooks = kNotTouching;
          auto down_hooks = kNotTouching;
          if (!sq) {
            across_hooks = hook_table_[Move::Across - 1][sq_row][sq_col];
            down_hooks = hook_table_[Move::Down - 1][sq_row][sq_col];
          }
          if ((num_tiles == 1) && (across_hooks != kNotTouching) &&
              (down_hooks != kNotTouching)) {
            if (direction == Move::Across) {
              // LOG(INFO) << "across_cross: "
              //           << tiles_.ToString(*across_cross).value();
              // LOG(INFO) << "down_cross: "
              //           << tiles_.ToString(*down_cross).value();
              recorded_num_tiles.insert(num_tiles);
              int tiles_before_first_played_tile = 0;
              const auto across_cross =
                  CrossAt(board, Move::Across, sq_row, sq_col);
              const auto down_cross =
                  CrossAt(board, Move::Down, sq_row, sq_col);
              if (down_cross->length() >= across_cross->length()) {
                for (Letter letter : down_cross.value()) {
                  if (letter) {
                    tiles_before_first_played_tile++;
                  } else {
                    break;
                  }
                }
                // LOG(INFO) << "pushing one tile play at " << sq_row << ",
                // "
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
                // LOG(INFO) << "pushing one tile play at " << sq_row << ",
                // "
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

int MoveFinder::SpotMaxScore(const LetterString& letters, const Board& board,
                             const Spot& spot) const {
  // LOG(INFO) << "SpotMaxScore(...)";
  const auto direction = spot.Direction();
  const int start_row = spot.StartRow();
  const int start_col = spot.StartCol();
  const int num_tiles = spot.NumTiles();
  const int word_multiplier = spot.WordMultiplier();

  std::array<int, 7> tile_multipliers;
  int row = start_row;
  int col = start_col;
  int min_multiplier = 99;
  int max_multiplier = 0;
  for (int i = 0; i < num_tiles;) {
    if (!board.At(row, col)) {
      const int letter_multiplier = board_layout_.LetterMultiplier(row, col);
      int tile_multiplier = word_multiplier * letter_multiplier;
      const auto cross = hook_table_[direction - 1][row][col];
      if (cross != kNotTouching) {
        tile_multiplier +=
            letter_multiplier * board_layout_.WordMultiplier(row, col);
      }
      min_multiplier = std::min(min_multiplier, tile_multiplier);
      max_multiplier = std::max(max_multiplier, tile_multiplier);
      tile_multipliers[i] = tile_multiplier;
      i++;
    }
    if (direction == Move::Across) {
      col++;
    } else {
      row++;
    }
  }
  std::array<int, 7> tile_scores;
  for (int i = 0; i < letters.size(); ++i) {
    tile_scores[i] = tiles_.Score(letters[i]);
  }
  if (min_multiplier != max_multiplier) {
    std::sort(tile_multipliers.begin(), tile_multipliers.begin() + num_tiles,
              std::greater<int>());
  }
  std::sort(tile_scores.begin(), tile_scores.begin() + letters.size(),
            std::greater<int>());

  int max_word_score = 0;
  for (int i = 0; i < num_tiles; i++) {
    max_word_score += tile_multipliers[i] * tile_scores[i];
  }
  // LOG(INFO) << "done";
  return max_word_score;
}

void MoveFinder::ComputeEmptyBoardSpotMaxEquity(const Rack& rack,
                                                const Board& board,
                                                Spot* spot) {
  // LOG(INFO) << "ComputeEmptyBoardSpotMaxEquity(...)";
  const int num_tiles = spot->NumTiles();
  const int word_multiplier = 2;
  spot->SetWordMultiplier(word_multiplier);
  const int hook_sum = 0;
  const int through_score = 0;
  const int bingo_bonus = num_tiles == 7 ? 50 : 0;
  const int extra_score = through_score + bingo_bonus + hook_sum;
  const int max_word_score = SpotMaxScore(rack.Letters(), board, *spot);
  const int leave_size = rack.NumTiles() - num_tiles;
  float best_leave = best_leave_at_size_[leave_size];

  spot->SetExtraScore(extra_score);
  spot->SetMaxScore(max_word_score + extra_score);
  spot->SetMaxEquity(max_word_score + extra_score + best_leave);
  // LOG(INFO) << "done";
}

void MoveFinder::ComputeSpotMaxEquity(const Rack& rack, const Board& board,
                                      Spot* spot) {
  const auto direction = spot->Direction();
  const int start_row = spot->StartRow();
  const int start_col = spot->StartCol();
  const int num_tiles = spot->NumTiles();
  const int word_multiplier =
      WordMultiplier(board, direction, start_row, start_col, num_tiles);
  spot->SetWordMultiplier(word_multiplier);
  const int hook_sum =
      HookSum(board, direction, start_row, start_col, num_tiles);
  const int through_score =
      ThroughScore(board, direction, start_row, start_col, num_tiles) *
      word_multiplier;
  const int bingo_bonus = num_tiles == 7 ? 50 : 0;
  const int extra_score = through_score + bingo_bonus + hook_sum;
  const int max_word_score = SpotMaxScore(rack.Letters(), board, *spot);
  const int leave_size = rack.NumTiles() - num_tiles;
  float best_leave = best_leave_at_size_[leave_size];

  spot->SetExtraScore(extra_score);
  spot->SetMaxScore(max_word_score + extra_score);
  spot->SetMaxEquity(max_word_score + extra_score + best_leave);
}

std::vector<MoveFinder::Spot> MoveFinder::FindSpots(const Rack& rack,
                                                    const Board& board) {
  // LOG(INFO) << "FindSpots(...)";
  std::vector<MoveFinder::Spot> spots;
  if (board.IsEmpty()) {
    spots.reserve(7 * 7 + 6 * 6 + 5 * 5 + 4 * 4 + 3 * 3 + 2 * 2);
    for (int num_tiles = 2; num_tiles <= rack.NumTiles(); num_tiles++) {
      for (int start_col = 7 + 1 - num_tiles; start_col <= 7; start_col++) {
        if (!rack_word_of_length_[num_tiles]) {
          continue;
        }
        Spot spot(Move::Across, 7, start_col, num_tiles);
        ComputeEmptyBoardSpotMaxEquity(rack, board, &spot);
        spots.push_back(spot);
      }
    }
  } else {
    FindSpots(rack.NumTiles(), board, Move::Across, &spots);
    FindSpots(rack.NumTiles(), board, Move::Down, &spots);
  }
  for (auto& spot : spots) {
    ComputeSpotMaxEquity(rack, board, &spot);
  }
  return spots;
}

std::vector<Move> MoveFinder::FindMoves(const Rack& rack, const Board& board,
                                        const Bag& bag,
                                        RecordMode record_mode) {
  // LOG(INFO) << "************************** FindMoves(...)";
  CacheCrossesAndScores(board);
  // LOG(INFO) << "Cached crosses and scores.";
  CacheRackPartitions(rack);
  // LOG(INFO) << "Cached rack partitions.";
  cross_map_.clear();
  subracks_.clear();
  std::vector<Move> moves;
  // In normal static eval, Pass 0 is a last resort.
  LetterString empty;
  Move pass(empty);
  pass.SetLeave(rack.Letters());
  pass.SetLeaveValue(-1000);
  pass.ComputeEquity();
  moves.push_back(pass);

  if (bag.CanExchange()) {
    const auto exchanges = FindExchanges(rack);
    if (record_mode == MoveFinder::RecordBest) {
      for (const Move& exchange : exchanges) {
        if (exchange.Equity() > moves[0].Equity()) {
          moves[0] = exchange;
        }
      }
    } else {
      moves.insert(moves.end(), exchanges.begin(), exchanges.end());
    }
  }
  std::vector<MoveFinder::Spot> spots = FindSpots(rack, board);
  // LOG(INFO) << "spots.size(): " << spots.size() << ", sorting...";
  if (record_mode == MoveFinder::RecordBest) {
    std::stable_sort(spots.begin(), spots.end(),
                     [](const Spot& a, const Spot& b) {
                       return a.MaxEquity() > b.MaxEquity();
                     });
  }
  // LOG(INFO) << "sorted spots";
  float best_equity = moves[0].Equity();
  int spots_checked = 0;
  for (const MoveFinder::Spot& spot : spots) {
    if (record_mode == MoveFinder::RecordBest) {
      if (spot.MaxEquity() < best_equity + 1e-5) {
        // LOG(INFO) << "Checked " << spots_checked << " out of " <<
        // spots.size()
        //           << " spots.";
        break;
      }
    }
    spots_checked++;

    // LOG(INFO) << "spot: " << spot.Direction() << ", " << spot.StartRow() <<
    // ", "
    //           << spot.StartCol() << ", " << spot.NumTiles();
    const auto words = FindWords(rack, board, spot, record_mode, best_equity);
    // LOG(INFO) << "words.size(): " << words.size();
    if (record_mode == MoveFinder::RecordBest) {
      for (const Move& word : words) {
        // LOG(INFO) << "word: " << tiles_.ToString(word.Letters()).value() <<
        // ", "
        //           << word.Equity();
        if (word.Equity() > moves[0].Equity()) {
          best_equity = word.Equity();
          moves[0] = word;
        }
      }
    } else {
      moves.insert(moves.end(), words.begin(), words.end());
    }
  }
  // LOG(INFO) << "Found " << moves.size() << " moves.";
  return moves;
}
