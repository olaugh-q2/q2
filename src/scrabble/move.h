#ifndef SRC_SCRABBLE_MOVE_H
#define SRC_SCRABBLE_MOVE_H

#include "absl/status/statusor.h"
#include "glog/logging.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Move {
 public:
  enum Action { Exchange, Place, OppDeadwoodBonus, OwnDeadwoodPenalty };
  enum Dir { None, Across, Down };

  Move() : action_(Move::Exchange) {}

  ~Move() {}

  Move(Dir direction, int start_row, int start_col, const LetterString& letters)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters),
        score_(absl::nullopt) {}

  Move(Dir direction, int start_row, int start_col, const LetterString& letters,
       int score)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters),
        score_(score) {}

  Move(Action action, const LetterString& letters, int score)
      : action_(action), letters_(letters), score_(score) {}

  explicit Move(const LetterString& letters)
      : action_(Move::Exchange), letters_(letters), score_(0) {}

  Move(const Move& move) {
    action_ = move.action_;
    direction_ = move.direction_;
    start_row_ = move.start_row_;
    start_col_ = move.start_col_;
    letters_ = move.letters_;
    score_ = move.score_;
    leave_ = move.leave_;
    leave_value_ = move.leave_value_;
    equity_ = move.equity_;
    num_tiles_ = move.num_tiles_;
    num_blanks_ = move.num_blanks_;
    move_bits_ = move.move_bits_;
  }

  const Move& operator=(const Move& move) {
    action_ = move.action_;
    direction_ = move.direction_;
    start_row_ = move.start_row_;
    start_col_ = move.start_col_;
    letters_ = move.letters_;
    score_ = move.score_;
    leave_ = move.leave_;
    leave_value_ = move.leave_value_;
    equity_ = move.equity_;
    num_tiles_ = move.num_tiles_;
    num_blanks_ = move.num_blanks_;
    move_bits_ = move.move_bits_;
    return *this;
  }

  Move& operator=(Move&& move) {
    action_ = move.action_;
    direction_ = move.direction_;
    start_row_ = move.start_row_;
    start_col_ = move.start_col_;
    letters_ = move.letters_;
    score_ = move.score_;
    leave_ = std::move(move.leave_);
    leave_value_ = move.leave_value_;
    equity_ = move.equity_;
    num_tiles_ = move.num_tiles_;
    num_blanks_ = move.num_blanks_;
    move_bits_ = move.move_bits_;
    return *this;
  }

  Move(Move&& move) {
    action_ = move.action_;
    direction_ = move.direction_;
    start_row_ = move.start_row_;
    start_col_ = move.start_col_;
    letters_ = std::move(move.letters_);
    score_ = move.score_;
    leave_ = std::move(move.leave_);
    leave_value_ = move.leave_value_;
    equity_ = move.equity_;
    num_tiles_ = move.num_tiles_;
    num_blanks_ = move.num_blanks_;
    move_bits_ = move.move_bits_;
  }

  void Display(const Tiles& tiles, std::ostream& os) const;

  void DisplayVerbose(const Tiles& tiles, std::ostream& os) const;

  static absl::StatusOr<Move> Parse(const std::string& move_string,
                                    const Tiles& tiles);

  Action GetAction() const { return action_; }

  const LetterString& Letters() const { return letters_; }

  Dir Direction() const { return direction_; }

  int StartRow() const { return start_row_; }

  int StartCol() const { return start_col_; }

  void SetScore(int score) { score_ = score; }

  void SetLeave(const LetterString& leave) { leave_ = leave; }

  void SetLeaveValue(double leave_value) { leave_value_ = leave_value; }

  int Score() const {
    CHECK(score_.has_value());
    return score_.value();
  }

  void ComputeEquity() {
    CHECK(score_.has_value());
    CHECK(leave_value_.has_value());
    equity_ = score_.value() + leave_value_.value();
  }

  const LetterString& Leave() const {
    CHECK(leave_.has_value());
    return leave_.value();
  }

  float LeaveValue() const {
    CHECK(leave_value_.has_value());
    return leave_value_.value();
  }

  void SetEquity(float equity) { equity_ = equity; }

  double Equity() const {
    CHECK(equity_.has_value());
    return equity_.value();
  }

  void WriteProto(const Tiles& tiles, q2::proto::Move* result) const;

  bool IsSubsetOf(const Tiles& tiles, std::array<int, 32>* rack_counts,
                  bool* copy_counts_decremented) const;

  int NumTiles() const {
    int ret = 0;
    for (const Letter letter : letters_) {
      if (letter) {
        ret++;
      }
    }
    return ret;
  }

  inline void SetCachedNumTiles(int num_tiles) { num_tiles_ = num_tiles; }

  inline int CachedNumTiles() const { return num_tiles_; }

  inline void SetNumBlanks(int num_blanks) { num_blanks_ = num_blanks; }

  inline int NumBlanks() const { return num_blanks_; }

  inline void SetMoveBits(uint32_t move_bits) { move_bits_ = move_bits; }

  inline uint32_t MoveBits() const { return move_bits_; }

 private:
  std::string StartingSquare() const;
  enum Action action_;
  enum Dir direction_;
  int start_row_;
  int start_col_;
  LetterString letters_;
  absl::optional<int> score_;
  absl::optional<LetterString> leave_;
  absl::optional<float> leave_value_;
  absl::optional<float> equity_;
  uint32_t move_bits_;
  int num_tiles_;
  int num_blanks_;
};

#endif  // SRC_SCRABBLE_MOVE_H