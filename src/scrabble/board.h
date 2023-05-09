#ifndef SRC_SCRABBLE_BOARD_H
#define SRC_SCRABBLE_BOARD_H

#include <array>

#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/move.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Board {
 public:
  Board();
  LetterString Row(int row_index) const;
  Letter At(int row, int col) const { return rows_[row][col]; }
  void UnsafePlaceMove(const Move& move);
  void UnsafeUndoMove(const Move& move);
  bool IsEmpty() const { return is_empty_; }

  LetterString MainWord(const Move& move, const Tiles& tiles) const;
  absl::optional<LetterString> CrossAt(const Move& move, const Tiles& tiles,
                                       int square_row, int square_col) const;
  std::vector<std::string> CrossWords(const Move& move,
                                      const Tiles& tiles) const;

  void WriteMoveWords(const Move& move, const Tiles& tiles,
                      q2::proto::Move* proto) const;

  void SetLetters(const std::vector<std::string>& letters, const Tiles& tiles);

 private:
  std::array<LetterString, 15> rows_;
  bool is_empty_;
};

#endif  // SRC_SCRABBLE_BOARD_H