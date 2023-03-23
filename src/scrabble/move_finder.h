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

typedef std::array<uint32_t, 15> HookRow;
typedef std::array<HookRow, 15> HookBoard;
typedef std::array<HookBoard, 2> HookTable;

typedef std::array<int, 15> ScoreRow;
typedef std::array<ScoreRow, 15> ScoreBoard;
typedef std::array<ScoreBoard, 2> ScoreTable;

// Special value for a tile that is not touching any other tiles.
// Distinct from a hypothetical hook spot that makes words with every letter,
// because we need to know that using this square is not sufficient to attach
// a word to the existing plays on the board.
const uint32_t kNotTouching = 0xffffffff;

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

    void SetWordMultiplier(int word_multiplier) {
      word_multiplier_ = word_multiplier;
    }
    void SetExtraScore(int extra_score) { extra_score_ = extra_score; }
    void SetMaxScore(int max_score) { max_score_ = max_score; }
    void SetMaxEquity(double max_equity) { max_equity_ = max_equity; }
    Move::Dir Direction() const { return direction_; }
    int StartRow() const { return start_row_; }
    int StartCol() const { return start_col_; }
    int NumTiles() const { return num_tiles_; }
    int WordMultiplier() const { return word_multiplier_; }
    int ExtraScore() const { return extra_score_; }
    int MaxScore() const { return max_score_; }
    double MaxEquity() const { return max_equity_; }

   private:
    Move::Dir direction_;
    int start_row_;
    int start_col_;
    int num_tiles_;
    int word_multiplier_;

    // extra_score = through score + hook sum + bingo bonus
    int extra_score_;

    // highest score, leave not added on
    int max_score_;

    // For now this is the highest score in this spot + the best leave, even if
    // they aren't compatible.
    double max_equity_;
  };

  MoveFinder(const AnagramMap& anagram_map, const BoardLayout& board_layout,
             const Tiles& tiles, const Leaves& leaves)
      : anagram_map_(anagram_map),
        board_layout_(board_layout),
        tiles_(tiles),
        leaves_(leaves) {}

  std::vector<Move> FindMoves(const Rack& rack, const Board& board,
                              const Bag& bag, RecordMode record_mode);
  std::vector<Move> FindExchanges(const Rack& rack) const;

 private:
  FRIEND_TEST(MoveFinderTest, Blankify);
  FRIEND_TEST(MoveFinderTest, BlankifyAllBlanks);
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
  FRIEND_TEST(MoveFinderTest, MemoizedCrossAt);
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
  FRIEND_TEST(MoveFinderTest, CacheCrossesAndScores);

  void ComputeSpotMaxEquity(const Rack& rack, const Board& board,
                            Spot* spot);

  void FindSpots(int rack_tiles, const Board& board, Move::Dir direction,
                 std::vector<MoveFinder::Spot>* spots);

  std::vector<Spot> FindSpots(const Rack& rack, const Board& board);

  absl::uint128 AbsorbThroughTiles(const Board& board, Move::Dir direction,
                                   int start_row, int start_col,
                                   int num_tiles) const;

  int WordMultiplier(const Board& board, Move::Dir direction, int start_row,
                     int start_col, int num_tiles) const;

  int HookSum(const Board& board, Move::Dir direction, int start_row,
              int start_col, int num_tiles) const;

  int ThroughScore(const Board& board, Move::Dir direction, int start_row,
                   int start_col, int num_tiles) const;

  int WordScore(const Board& board, const Move& move, int word_multiplier);

  // Returns true iff the word fits with the tiles already on the board.
  bool FitsWithPlayedThroughTiles(const Board& board, Move::Dir direction,
                                  int start_row, int start_col,
                                  const LetterString& word) const;

  LetterString ZeroPlayedThroughTiles(const Board& board, Move::Dir direction,
                                      int start_row, int start_col,
                                      const LetterString& word) const;

  void CacheCrossesAndScores(const Board& board);

  absl::optional<LetterString> MemoizedCrossAt(const Board& board,
                                               Move::Dir play_dir,
                                               int square_row, int square_col,
                                               bool unblank);

  // Returns nullopt if the square is unconstrained, otherwise returns a string
  // with a gap for this square's hooks which is a key to use with
  // anagram_map_.Hooks(). Set unblank to true for checking hook validity; set
  // it to false to get tiles for scoring.
  absl::optional<LetterString> CrossAt(const Board& board, Move::Dir play_dir,
                                       int square_row, int square_col,
                                       bool unblank) const;
  bool CheckHooks(const Board& board, const Move& move);

  // Word should only have played tiles (play-through is zeroed out).
  // Returns copies of the word with blanks designated to make legal plays.
  std::vector<LetterString> Blankify(const LetterString& rack_letters,
                                     const LetterString& word) const;

  const std::vector<std::pair<absl::uint128, LetterString>>& Subracks(
      const Rack& rack, int num_tiles);

  std::vector<Move> FindWords(const Rack& rack, const Board& board,
                              const Spot& spot, RecordMode record_mode,
                              double best_equity);

  const AnagramMap& anagram_map_;
  const BoardLayout& board_layout_;
  const Tiles& tiles_;
  const Leaves& leaves_;
  std::vector<std::pair<absl::uint128, LetterString>> subracks_;

  absl::flat_hash_map<std::tuple<Move::Dir, int, int, bool>,
                      absl::optional<LetterString>>
      cross_map_;

  HookTable hook_table_;
  ScoreTable score_table_;
};

inline bool operator==(const MoveFinder::Spot& a, const MoveFinder::Spot& b) {
  return a.Direction() == b.Direction() && a.StartRow() == b.StartRow() &&
         a.StartCol() == b.StartCol() && a.NumTiles() == b.NumTiles();
}

#endif  // SRC_SCRABBLE_MOVE_FINDER_H_