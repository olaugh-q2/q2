#include "src/scrabble/move.h"

#include "absl/strings/ascii.h"
#include "glog/logging.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

std::string Move::StartingSquare() const {
  if (direction_ == Move::None) {
    LOG(FATAL) << "Invalid direction for move.";
  }
  std::stringstream ss;
  if (direction_ == Move::Across) {
    ss << static_cast<int>(start_row_ + 1);
    ss << static_cast<char>('A' + start_col_);
  } else {
    ss << static_cast<char>('A' + start_col_);
    ss << static_cast<int>(start_row_ + 1);
  }
  return ss.str();
}

void Move::Display(const Tiles& tiles, std::ostream& os) const {
  // LOG(INFO) << "Displaying move";
  if (action_ == Move::Exchange) {
    // LOG(INFO) << "letters_.size() = " << letters_.size();
    if (letters_.empty()) {
      std::string pass("PASS 0");
      // LOG(INFO) << "pass: " << pass;
      os << pass;
    } else {
      std::string exch("EXCH ");
      // LOG(INFO) << "exch: " << exch;
      os << exch << tiles.ToString(letters_).value();
    }
  } else {
    os << StartingSquare() << " ";
    os << tiles.ToString(letters_).value();
  }
  if (score_) {
    os << " (score = " << score_.value() << ")";
  }
}

void Move::DisplayVerbose(const Tiles& tiles, std::ostream& os) const {
  // LOG(INFO) << "Displaying move";
  if (action_ == Move::Exchange) {
    // LOG(INFO) << "letters_.size() = " << letters_.size();
    if (letters_.empty()) {
      std::string pass("PASS 0");
      // LOG(INFO) << "pass: " << pass;
      os << pass;
    } else {
      std::string exch("EXCH ");
      // LOG(INFO) << "exch: " << exch;
      os << exch << tiles.ToString(letters_).value();
    }
  } else {
    os << StartingSquare() << " ";
    os << tiles.ToString(letters_).value();
  }
  if (leave_ && leave_value_) {
    if (leave_.value().size() == 0) {
      os << " [leaving nothing = " << leave_value_.value() << "]";
    } else {
      os << " [leaving " << tiles.ToString(*leave_).value() << " = "
         << leave_value_.value() << "]";
    }
  }
  if (score_) {
    os << " (score = " << score_.value() << ")";
  }
  if (equity_) {
    os << " (equity = " << equity_.value() << ")";
  }
}

absl::StatusOr<Move> Move::Parse(const std::string& move_string,
                                 const Tiles& tiles) {
  std::string upcase_string = absl::AsciiStrToUpper(move_string);
  if (move_string.empty()) {
    return absl::InvalidArgumentError("Move string is empty.");
  }
  if (move_string[0] == '-') {
    if (move_string.size() == 1) {
      const LetterString empty;
      return Move(empty);
    } else {
      const auto letters = tiles.ToLetterString(move_string.substr(1));
      if (!letters) {
        return absl::InvalidArgumentError("Invalid exchange move: " +
                                          move_string);
      } else {
        return Move(tiles.Unblank(letters.value()));
      }
    }
  }
  if (upcase_string.substr(0, 4) == "PASS") {
    LetterString empty;
    Move pass(empty);
    Move ret = std::move(pass);
    return ret;
  }
  if (upcase_string.substr(0, 5) == "EXCH ") {
    const auto letters = tiles.ToLetterString(move_string.substr(5));
    if (!letters) {
      return absl::InvalidArgumentError("Invalid exchange move: " +
                                        move_string);
    } else {
      return Move(tiles.Unblank(letters.value()));
    }
  }
  if (move_string.size() < 5) {
    return absl::InvalidArgumentError("Invalid move: " + move_string);
  }
  enum Dir direction;
  int display_start_row;
  int start_col;
  int starting_square_length = 3;  // including space
  if (upcase_string[0] >= '1' && upcase_string[0] <= '9') {
    direction = Move::Across;
    display_start_row = upcase_string[0] - '0';
    if (upcase_string[1] >= '0' && upcase_string[1] <= '9') {
      display_start_row = 10 * display_start_row + upcase_string[1] - '0';
      start_col = upcase_string[2] - 'A';
      ++starting_square_length;
    } else {
      start_col = upcase_string[1] - 'A';
    }
  } else {
    direction = Move::Down;
    start_col = upcase_string[0] - 'A';
    display_start_row = upcase_string[1] - '0';
    if (upcase_string[2] >= '0' && upcase_string[2] <= '9') {
      ++starting_square_length;
      display_start_row = 10 * display_start_row + upcase_string[2] - '0';
    }
  }
  const int start_row = display_start_row - 1;
  if (start_row < 0 || start_row > 14 || start_col < 0 || start_col > 14) {
    return absl::InvalidArgumentError("Invalid starting square for move: " +
                                      move_string);
  }
  const auto letters =
      tiles.ToLetterString(move_string.substr(starting_square_length));
  if (!letters) {
    return absl::InvalidArgumentError("Invalid tiles for place move: " +
                                      move_string);
  }
  return Move(direction, start_row, start_col, letters.value());
}