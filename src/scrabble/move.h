#ifndef SRC_SCRABBLE_MOVE_H_
#define SRC_SCRABBLE_MOVE_H_

#include "absl/status/statusor.h"
#include "glog/logging.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Move {
 public:
  enum Action { Exchange, Place };
  enum Dir { None, Across, Down };

  Move() : action_(Move::Exchange) {}

  ~Move() {}

  Move(Dir direction, int start_row, int start_col, const LetterString& letters)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters), score_(absl::nullopt) {}

  Move(Dir direction, int start_row, int start_col, const LetterString& letters,
       int score)
      : action_(Move::Place),
        direction_(direction),
        start_row_(start_row),
        start_col_(start_col),
        letters_(letters),
        score_(score) {}

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
  }

  void Display(const Tiles& tiles, std::ostream& os) const;

  void DisplayVerbose(const Tiles& tiles, std::ostream& os) const;

  static absl::StatusOr<Move> Parse(const std::string& move_string,
                                    const Tiles& tiles);

  Action Action() const { return action_; }

  LetterString Letters() const { return letters_; }

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

  LetterString Leave() const {
    CHECK(leave_.has_value());
    return leave_.value();
  }

  double LeaveValue() const {
    CHECK(leave_value_.has_value());
    return leave_value_.value();
  }

  double Equity() const {
    CHECK(equity_.has_value());
    return equity_.value();
  }

 private:
  std::string StartingSquare() const;

  enum Action action_;
  enum Dir direction_;
  int start_row_;
  int start_col_;
  LetterString letters_;
  absl::optional<int> score_;
  absl::optional<LetterString> leave_;
  absl::optional<double> leave_value_;
  absl::optional<double> equity_;
};

#endif  // SRC_SCRABBLE_MOVE_H_