#ifndef SRC_SCRABBLE_MOVE_H_
#define SRC_SCRABBLE_MOVE_H_

#include "absl/status/statusor.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Move {
 public:
  enum Action { Exchange, Place };
  enum Dir { None, Across, Down };

  Move() : action_(Move::Exchange), letters_(LetterString()) {}

  Move(Dir direction, int start_row, int start_col,
       const LetterString& letters)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters) {}

  Move(const LetterString& letters)
      : action_(Move::Exchange), letters_(letters) {}

  void Display(const Tiles& tiles, std::ostream& os) const;

  static absl::StatusOr<Move> Parse(const std::string& move_string,
                                    const Tiles& tiles);

  Action Action() const { return action_; }

  LetterString Letters() const { return letters_; }

  Dir Direction() const { return direction_; }

  int StartRow() const { return start_row_; }

  int StartCol() const { return start_col_; }

 private:
  std::string StartingSquare() const;

  enum Action action_;
  enum Dir direction_;
  int start_row_;
  int start_col_;
  LetterString letters_;
};

#endif  // SRC_SCRABBLE_MOVE_H_