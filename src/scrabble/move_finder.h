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
          num_tiles_(num_tiles),
          minimum_blanks_(0),
          required_letters_(0),
          impossible_letters_(0) {}

    uint16_t ThroughScore() const { return through_score_; }
    void SetThroughScore(int through_score) { through_score_ = through_score; }
    inline void SetWordMultiplier(int word_multiplier) {
      word_multiplier_ = word_multiplier;
    }
    inline void SetExtraScore(int extra_score) { extra_score_ = extra_score; }
    inline void SetMaxScore(int max_score) { max_score_ = max_score; }
    inline void SetMaxEquity(float max_equity) { max_equity_ = max_equity; }
    inline Move::Dir Direction() const { return direction_; }
    inline uint8_t StartRow() const { return start_row_; }
    inline uint8_t StartCol() const { return start_col_; }
    inline uint8_t NumTiles() const { return num_tiles_; }
    inline uint8_t WordMultiplier() const { return word_multiplier_; }
    inline uint16_t ExtraScore() const { return extra_score_; }
    inline uint16_t MaxScore() const { return max_score_; }
    inline float MaxEquity() const { return max_equity_; }

    inline void SetMinimumBlanks(uint8_t minimum_blanks) {
      minimum_blanks_ = minimum_blanks;
    }
    inline void SetRequiredLetters(uint32_t required_letters) {
      required_letters_ = required_letters;
    }
    inline void SetImpossibleLetters(uint32_t impossible_letters) {
      impossible_letters_ = impossible_letters;
    }
    inline bool CompatibleWith(uint32_t letters, uint8_t num_blanks) const {
      /*
      std::string letter_string;
      std::string impossible_letters;
      std::string required_letters;
      for (int i = 1; i <= 27; ++i) {
        char letter = 'A' + i - 1;
        if (i == 27) {
          letter = '?';
        }
        if (letters & (1 << i)) {
          letter_string += letter;
        }
        if (impossible_letters_ & (1 << i)) {
          impossible_letters += letter;
        }
        if (required_letters_ & (1 << i)) {
          required_letters += letter;
        }
      }
      */
      // LOG(INFO) << "CompatibleWith(...) letters: " << letter_string << "
      //  impossible: "
      //            << impossible_letters << " required: " << required_letters;
      /*
      if (letters & impossible_letters_) {
        LOG(INFO) << "Impossible letters";
      }
      if (num_blanks < minimum_blanks_) {
        LOG(INFO) << "Not enough blanks";
      }
      if ((letters & required_letters_) != required_letters_) {
        LOG(INFO) << "Missing required letters";
      }
      */
      return ((letters & impossible_letters_) == 0) &&
             ((letters & required_letters_) == required_letters_) &&
             (num_blanks >= minimum_blanks_);
    }

   private:
    Move::Dir direction_;
    uint8_t start_row_;
    uint8_t start_col_;
    uint8_t num_tiles_;
    uint8_t word_multiplier_;

    // extra_score = through score + hook sum + bingo bonus
    uint16_t extra_score_;

    // highest score, leave not added on
    uint16_t max_score_;

    uint16_t through_score_;

    // For now this is the highest score in this spot + the best leave, even if
    // they aren't compatible.
    float max_equity_;

    uint8_t minimum_blanks_;
    uint32_t required_letters_;
    uint32_t impossible_letters_;
  };

  class RackPartition {
   public:
    RackPartition(uint64_t used_product, uint8_t num_blanks,
                  const LetterString& used_letters,
                  const LetterString& left_letters, float leave_value,
                  uint32_t bits, AnagramMapIterator word_iterator)
        : used_product_(used_product),
          num_blanks_(num_blanks),
          used_letters_(used_letters),
          left_letters_(left_letters),
          leave_value_(leave_value),
          has_word_status_(kHasWord),
          word_iterator_(word_iterator),
          bits_(bits) {}
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
      // LOG(INFO) << "SetWordIterator called";
      word_iterator_ = word_iterator;
    }
    const AnagramMapIterator& WordIterator() const { return word_iterator_; }
    uint32_t Bits() const { return bits_; }

   private:
    // Prime product of the letters in used_letters_, not including blanks.
    uint64_t used_product_;

    uint8_t num_blanks_;
    LetterString used_letters_;
    LetterString left_letters_;
    float leave_value_;
    enum HasWordStatus has_word_status_;
    AnagramMapIterator word_iterator_;
    uint32_t bits_;
  };

  MoveFinder(const AnagramMap& anagram_map, const BoardLayout& board_layout,
             const Tiles& tiles, const Leaves& leaves)
      : anagram_map_(anagram_map),
        board_layout_(board_layout),
        tiles_(tiles),
        leaves_(leaves) {
    // LOG(INFO) << "MoveFinder constructor called";
    moves_.reserve(80000);
    ClearHookTables();
  }

  const std::vector<Move>& Moves() const { return moves_; }
  void FindMoves(const Rack& rack, const Board& board, const Bag& bag,
                 RecordMode record_mode, bool recompute_all_crosses_and_scores);
  void CacheSubsets(const Rack& rack);
  void CacheRackPartitions(const Rack& rack);
  std::vector<Move> FindExchanges(const Rack& rack,
                                  RecordMode record_mode) const;

  void CacheCrossesAndScores(const Board& board, int row, int col, bool across,
                             bool down);
  void CacheCrossesAndScores(const Board& board, const Move& move);

  void CacheCrossesAndScores(const Board& board);

  bool IsBlocked(const Move& move, const Board& board) const;

  void ClearHookTables() {
    // LOG(INFO) << "clearing hook tables";
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 15; ++j) {
        for (int k = 0; k < 15; ++k) {
          hook_table_[i][j][k] = kNotTouching;
          score_table_[i][j][k] = 0;
        }
      }
    }
  }


  void SetEndgameEquities(const LetterString& rack,
                          const LetterString& opp_rack, bool preceded_by_pass,
                          const Tiles& tiles);

  void SortMoves();

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

  void ComputeEmptyBoardSpotMaxEquity(const Rack& rack, Spot* spot);
  void ComputeSpotMaxEquity(const Rack& rack,
                            const std::array<int, 7>& sorted_tile_scores,
                            const Board& board, Spot* spot);

  void FindSpots(int rack_tiles, const Board& board, Move::Dir direction,
                 std::vector<MoveFinder::Spot>* spots);

  void FindSpots(const Rack& rack, const Board& board,
                 std::vector<Spot>* spots);
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

  void Blankify(const LetterString& rack_letters, const LetterString& word,
                std::vector<LetterString>* target) const;

  // Word should only have played tiles (play-through is zeroed out).
  // Returns copies of the word with blanks designated to make legal plays.
  std::vector<LetterString> Blankify(const LetterString& rack_letters,
                                     const LetterString& word) const;

  int EmptyBoardSpotMaxScore(const LetterString& letters,
                             const Spot& spot) const;
  int SpotMaxScore(const std::array<int, 7>& sorted_tile_scores,
                   const Board& board, const Spot& spot) const;

  bool HasWord(RackPartition* partition) const {
    // LOG(INFO) << "HasWord: " << partition->UsedProduct() << " "
    //           << static_cast<int>(partition->NumBlanks()) << " "
    //           << tiles_.ToString(partition->UsedLetters()).value();
    if (partition->GetHasWordStatus() == kHasWord) {
      // LOG(INFO) << "cached true";
      return true;
    } else if (partition->GetHasWordStatus() == kNoWord) {
      // LOG(INFO) << "cached false";
      return false;
    } else {
      // LOG(INFO) << "setting word iterator";
      partition->SetWordIterator(anagram_map_.WordIterator(
          partition->UsedProduct(), partition->NumBlanks()));
      if (anagram_map_.HasWord(partition->WordIterator())) {
        // LOG(INFO) << "looked up, true";
        partition->SetHasWordStatus(kHasWord);
        return true;
      } else {
        // LOG(INFO) << "looked up, false";
        partition->SetHasWordStatus(kNoWord);
        return false;
      }
    }
  }

  std::vector<Move> FindWords(const Rack& rack, const Board& board,
                              const Spot& spot, RecordMode record_mode,
                              float best_equity);

  void FindWords(const Rack& rack, const Board& board, const Spot& spot,
                 RecordMode record_mode, float best_equity,
                 std::vector<Move>* moves);

  void SetRackBits(const Rack& rack);

  const AnagramMap& anagram_map_;
  const BoardLayout& board_layout_;
  const Tiles& tiles_;
  const Leaves& leaves_;
  std::vector<std::pair<absl::uint128, LetterString>> subracks_;

  absl::flat_hash_map<std::tuple<Move::Dir, int, int, bool>,
                      absl::optional<LetterString>>
      cross_map_;
  absl::flat_hash_map<uint64_t, LetterString> subsets_;
  HookTable hook_table_;
  ScoreTable score_table_;
  std::vector<Move> moves_;
  std::vector<LetterString> blankified_words_;

  // For each size from 0 to 7 tiles, a list of possible divisions of the rack
  // into used and left tiles, with data needed for finding and evaluating
  // moves.
  std::array<std::vector<RackPartition>, 8> rack_partitions_;

  std::array<float, 8> best_leave_at_size_;
  std::array<bool, 8> rack_word_of_length_;
  uint32_t rack_bits_;
  uint32_t unique_rack_letter_bits_;
  int num_blanks_;
  std::vector<Spot> spots_;
  std::vector<const Spot*> spot_ptrs_;
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