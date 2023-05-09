#ifndef SRC_SCRABBLE_MOVE_FINDER_H
#define SRC_SCRABBLE_MOVE_FINDER_H

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
  enum HasWordStatus { kHasWord, kNoWord, kUnchecked };
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
    void SetMaxEquity(float max_equity) { max_equity_ = max_equity; }
    Move::Dir Direction() const { return direction_; }
    int StartRow() const { return start_row_; }
    int StartCol() const { return start_col_; }
    int NumTiles() const { return num_tiles_; }
    int WordMultiplier() const { return word_multiplier_; }
    int ExtraScore() const { return extra_score_; }
    int MaxScore() const { return max_score_; }
    float MaxEquity() const { return max_equity_; }

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
    float max_equity_;
  };

  class RackPartition {
   public:
    RackPartition(uint64_t used_product, uint8_t num_blanks,
                  const LetterString& used_letters,
                  const LetterString& left_letters, float leave_value,
                  AnagramMapIterator word_iterator)
        : used_product_(used_product),
          num_blanks_(num_blanks),
          used_letters_(used_letters),
          left_letters_(left_letters),
          leave_value_(leave_value),
          has_word_status_(kHasWord),
          word_iterator_(word_iterator)
           {}
    uint64_t UsedProduct() const { return used_product_; }
    uint8_t NumBlanks() const { return num_blanks_; }
    const LetterString& UsedLetters() const { return used_letters_; }
    const LetterString& LeftLetters() const { return left_letters_; }
    float LeaveValue() const { return leave_value_; }
    HasWordStatus GetHasWordStatus() const { return has_word_status_; }
    void SetHasWordStatus(HasWordStatus has_word_status) {
      has_word_status_ = has_word_status;
    }
    void SetWordIterator(AnagramMapIterator word_iterator) {
      //LOG(INFO) << "SetWordIterator called";
      word_iterator_ = word_iterator;
    }
    const AnagramMapIterator& WordIterator() const { return word_iterator_; }

   private:
    // Prime product of the letters in used_letters_, not including blanks.
    uint64_t used_product_;

    uint8_t num_blanks_;
    LetterString used_letters_;
    LetterString left_letters_;
    float leave_value_;
    enum HasWordStatus has_word_status_;
    AnagramMapIterator word_iterator_;
  };

  MoveFinder(const AnagramMap& anagram_map, const BoardLayout& board_layout,
             const Tiles& tiles, const Leaves& leaves)
      : anagram_map_(anagram_map),
        board_layout_(board_layout),
        tiles_(tiles),
        leaves_(leaves) {
    //LOG(INFO) << "MoveFinder constructor called";         
    moves_.reserve(80000);
    ClearHookTables();
  }

  const std::vector<Move>& Moves() const { return moves_; }
  void FindMoves(const Rack& rack, const Board& board, const Bag& bag,
                 RecordMode record_mode, bool recompute_all_crosses_and_scores);
  std::vector<Move> FindExchanges(const Rack& rack) const;

  void CacheCrossesAndScores(const Board& board, int row, int col, bool across, bool down);
  void CacheCrossesAndScores(const Board& board, const Move& move);

  void CacheCrossesAndScores(const Board& board);

  bool IsBlocked(const Move& move, const Board& board) const;

  void ClearHookTables() {
    //LOG(INFO) << "clearing hook tables";
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 15; ++j) {
        for (int k = 0; k < 15; ++k) {
          hook_table_[i][j][k] = kNotTouching;
          score_table_[i][j][k] = 0;
        }
      }
    }
  }

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

  void ComputeEmptyBoardSpotMaxEquity(const Rack& rack, const Board& board,
                                      Spot* spot);
  void ComputeSpotMaxEquity(const Rack& rack, const Board& board, Spot* spot);

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

  // Returns nullopt if the square is unconstrained, otherwise returns a string
  // with a gap for this square's hooks which (after unblanking) is a key to use
  // with anagram_map_.Hooks(). To use this for scoring, don't unblank.
  absl::optional<LetterString> CrossAt(const Board& board, Move::Dir play_dir,
                                       int square_row, int square_col) const;
  bool CheckHooks(const Board& board, const Move& move) const;

  // Word should only have played tiles (play-through is zeroed out).
  // Returns copies of the word with blanks designated to make legal plays.
  std::vector<LetterString> Blankify(const LetterString& rack_letters,
                                     const LetterString& word) const;

  int SpotMaxScore(const LetterString& letters, const Board& board,
                   const Spot& spot) const;

  void CacheRackPartitions(const Rack& rack);
  bool HasWord(RackPartition* partition) const {
    //LOG(INFO) << "HasWord: " << partition->UsedProduct() << " "
    //          << static_cast<int>(partition->NumBlanks()) << " "
    //          << tiles_.ToString(partition->UsedLetters()).value();
    if (partition->GetHasWordStatus() == kHasWord) {
      //LOG(INFO) << "cached true";
      return true;
    } else if (partition->GetHasWordStatus() == kNoWord) {
      //LOG(INFO) << "cached false";
      return false;
    } else {
      //LOG(INFO) << "setting word iterator";
      partition->SetWordIterator(anagram_map_.WordIterator(
          partition->UsedProduct(), partition->NumBlanks()));
      if (anagram_map_.HasWord(partition->WordIterator())) {
        //LOG(INFO) << "looked up, true";
        partition->SetHasWordStatus(kHasWord);
        return true;
      } else {
        //LOG(INFO) << "looked up, false";
        partition->SetHasWordStatus(kNoWord);
        return false;
      }
    }
  }

  std::vector<Move> FindWords(const Rack& rack, const Board& board,
                              const Spot& spot, RecordMode record_mode,
                              float best_equity);

  void SetRackBits(const Rack& rack);
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
  std::vector<Move> moves_;

  // For each size from 0 to 7 tiles, a list of possible divisions of the rack
  // into used and left tiles, with data needed for finding and evaluating
  // moves.
  std::array<std::vector<RackPartition>, 8> rack_partitions_;

  std::array<float, 8> best_leave_at_size_;
  std::array<bool, 8> rack_word_of_length_;
  uint32_t rack_bits_;
  uint32_t unique_rack_letter_bits_;
  int num_blanks_;
};

inline bool operator==(const MoveFinder::Spot& a, const MoveFinder::Spot& b) {
  return a.Direction() == b.Direction() && a.StartRow() == b.StartRow() &&
         a.StartCol() == b.StartCol() && a.NumTiles() == b.NumTiles();
}

inline void MoveFinder::SetRackBits(const Rack& rack) {
  rack_bits_ = 0;
  num_blanks_ = 0;
  for (const Letter letter : rack.Letters()) {
    if (letter == tiles_.BlankIndex()) {
      num_blanks_++;
    } else {
      uint32_t bit = 1 << letter;
      if (rack_bits_ & bit) {
        unique_rack_letter_bits_ &= ~bit;
      } else {
        unique_rack_letter_bits_ |= bit;
      }
      rack_bits_ |= bit;
    }
  }
}

#endif  // SRC_SCRABBLE_MOVE_FINDER_H