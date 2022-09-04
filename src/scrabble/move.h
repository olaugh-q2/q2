#ifndef SRC_SCRABBLE_MOVE_H_
#define SRC_SCRABBLE_MOVE_H_

#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Move {
 public:
  enum Action { Exchange, Place };
  enum Direction { None, Across, Down };

  Move() : action_(Move::Exchange), letters_(LetterString()) {}

  Move(Direction direction, int start_row, int start_col,
       const LetterString& letters)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters) {}

  Move(const LetterString& letters)
      : action_(Move::Exchange), letters_(letters) {}

  void Display(const Tiles& tiles, std::ostream& os) const;

 private:
  std::string StartingSquare() const;

  Action action_;
  Direction direction_;
  int start_row_;
  int start_col_;
  LetterString letters_;
};

#endif  // SRC_SCRABBLE_MOVE_H_