#ifndef SRC_SCRABBLE_MOVE_FINDER_H_
#define SRC_SCRABBLE_MOVE_FINDER_H_

#include "src/anagram/anagram_map.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/move.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

class MoveFinder {
 public:
  MoveFinder(const AnagramMap& anagram_map, const BoardLayout& board_layout,
             const Tiles& tiles)
      : anagram_map_(anagram_map), board_layout_(board_layout), tiles_(tiles) {}

  std::vector<Move> FindExchanges(const Rack& rack) const;

 private:
  FRIEND_TEST(MoveFinderTest, Blankify);
  FRIEND_TEST(MoveFinderTest, FindWords);
  FRIEND_TEST(MoveFinderTest, FindWords2);
  FRIEND_TEST(MoveFinderTest, FindWords3);
  FRIEND_TEST(MoveFinderTest, FindWords4);
  FRIEND_TEST(MoveFinderTest, ZeroPlayedThroughTiles);
  FRIEND_TEST(MoveFinderTest, ZeroPlayedThroughTilesVertical);
  FRIEND_TEST(MoveFinderTest, PlayThroughWithBlanks);
  FRIEND_TEST(MoveFinderTest, AbsorbThroughTiles);
  FRIEND_TEST(MoveFinderTest, AbsorbThroughTiles2);

  absl::uint128 AbsorbThroughTiles(const Board& board, Move::Dir direction,
                                   int start_row, int start_col,
                                   int num_tiles) const;

  // "Zeroes out" the tiles on the board that are played through.
  // E.g. SHOESTRING played as SH(OESTRIN)G becomes SH.......G
  // Returns nullopt if the word does not fit with the tiles already on the
  // board.
  absl::optional<LetterString> ZeroPlayedThroughTiles(
      const Board& board, Move::Dir direction, int start_row, int start_col,
      const LetterString& word) const;

  // Word should only have played tiles (play-through is zeroed out).
  // Returns copies of the word with blanks designated to make legal plays.
  std::vector<LetterString> Blankify(const LetterString& rack_letters,
                                     const LetterString& word) const;

  std::vector<Move> FindWords(const Rack& rack, const Board& board,
                              Move::Dir direction, int start_row, int start_col,
                              int num_tiles) const;

  const AnagramMap& anagram_map_;
  const BoardLayout& board_layout_;
  const Tiles& tiles_;
};

#endif  // SRC_SCRABBLE_MOVE_FINDER_H_