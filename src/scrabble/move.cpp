#include "src/scrabble/move.h"

#include "glog/logging.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

std::string Move::StartingSquare() const {
  if (direction_ == Move::None) {
    LOG(FATAL) << "Invalid direction for move.";
  }
  std::string square;
  if (direction_ == Move::Across) {
    square += '1' + start_row_;
    square += 'A' + start_col_;
  } else {
    square += 'A' + start_col_;
    square += '1' + start_row_;
  }
  return square;
}

void Move::Display(const Tiles& tiles, std::ostream& os) const {
    LOG(INFO) << "Displaying move";
  if (action_ == Move::Exchange) {
    LOG(INFO) << "letters_.size() = " << letters_.size();
    if (letters_.empty()) {
      std::string pass("PASS 0");
      LOG(INFO) << "pass: " << pass;
      os << pass;
    } else {
      std::string exch("EXCH ");
      LOG(INFO) << "exch: " << exch;
      os << exch << tiles.ToString(letters_).value();
    }
  } else {
    os << StartingSquare() << " ";
    os << tiles.ToString(letters_).value();
  }
}