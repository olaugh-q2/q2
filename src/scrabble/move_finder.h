#ifndef SRC_SCRABBLE_MOVE_FINDER_H_
#define SRC_SCRABBLE_MOVE_FINDER_H_

#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/move.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

class MoveFinder {
 public:
  enum RecordMode { RecordAll, RecordBest, RecordBestK };
  class Spot {
   public:
    Spot(Move::Dir direction, int start_row, int start_col, int num_tiles)
        : direction_(direction),
          start_row_(start_row),
          start_col_(start_col),
          num_tiles_(num_tiles) {}

    Move::Dir Direction() const { return direction_; }
    int StartRow() const { return start_row_; }
    int StartCol() const { return start_col_; }
    int NumTiles() const { return num_tiles_; }

   private:
    Move::Dir direction_;
    int start_row_;
    int start_col_;
    int num_tiles_;
  };

  MoveFinder(const AnagramMap& anagram_map, const BoardLayout& board_layout,
             const Tiles& tiles, const Leaves& leaves)
      : anagram_map_(anagram_map), board_layout_(board_layout), tiles_(tiles), leaves_(leaves) {}

  std::vector<Move> FindMoves(const Rack& rack, const Board& board,
                              const Bag& bag, RecordMode record_mode) const;
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
  FRIEND_TEST(MoveFinderTest, CrossAt);
  FRIEND_TEST(MoveFinderTest, CheckHooks);
  FRIEND_TEST(MoveFinderTest, SevenTileOverlap);
  FRIEND_TEST(MoveFinderTest, NonHooks);
  FRIEND_TEST(MoveFinderTest, FrontExtension);
  FRIEND_TEST(MoveFinderTest, EmptyBoardSpots);
  FRIEND_TEST(MoveFinderTest, AcrossSpots);
  FRIEND_TEST(MoveFinderTest, DownSpots);
  FRIEND_TEST(MoveFinderTest, WordMultiplier);
  FRIEND_TEST(MoveFinderTest, HookSum);
  FRIEND_TEST(MoveFinderTest, ThroughScore);
  FRIEND_TEST(MoveFinderTest, WordScore);

  void FindSpots(int rack_tiles, const Board& board, Move::Dir direction,
                 std::vector<MoveFinder::Spot>* spots) const;

  std::vector<Spot> FindSpots(const Rack& rack, const Board& board) const;

  absl::uint128 AbsorbThroughTiles(const Board& board, Move::Dir direction,
                                   int start_row, int start_col,
                                   int num_tiles) const;

  int WordMultiplier(const Board& board, Move::Dir direction, int start_row,
                     int start_col, int num_tiles) const;

  int HookSum(const Board& board, Move::Dir direction, int start_row,
              int start_col, int num_tiles) const;

  int ThroughScore(const Board& board, Move::Dir direction, int start_row,
                   int start_col, int num_tiles) const;

  int WordScore(const Board& board, const Move& move,
                int word_multiplier) const;

  // "Zeroes out" the tiles on the board that are played through.
  // E.g. SHOESTRING played as SH(OESTRIN)G becomes SH.......G
  // Returns nullopt if the word does not fit with the tiles already on the
  // board.
  absl::optional<LetterString> ZeroPlayedThroughTiles(
      const Board& board, Move::Dir direction, int start_row, int start_col,
      const LetterString& word) const;

  // Returns nullopt if the square is unconstrained, otherwise returns a string
  // with a gap for this square's hooks which is a key to use with
  // anagram_map_.Hooks(). Set unblank to true for checking hook validity; set
  // it to false to get tiles for scoring.
  absl::optional<LetterString> CrossAt(const Board& board, Move::Dir play_dir,
                                       int square_row, int square_col,
                                       bool unblank) const;
  bool CheckHooks(const Board& board, const Move& move) const;

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
  const Leaves& leaves_;
};

inline bool operator==(const MoveFinder::Spot& a, const MoveFinder::Spot& b) {
  return a.Direction() == b.Direction() && a.StartRow() == b.StartRow() &&
         a.StartCol() == b.StartCol() && a.NumTiles() == b.NumTiles();
}

#endif  // SRC_SCRABBLE_MOVE_FINDER_H_