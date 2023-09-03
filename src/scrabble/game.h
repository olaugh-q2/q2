#ifndef SRC_SCRABBLE_GAME_H
#define SRC_SCRABBLE_GAME_H

#include "absl/random/random.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/player.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

class Game {
 public:
  Game(const BoardLayout& layout, const std::vector<Player*>& players,
       const Tiles& tiles, absl::Duration initial_time, std::size_t game_index)
      : layout_(layout),
        players_(players),
        tiles_(tiles),
        initial_time_(initial_time),
        pregame_bag_(Bag(tiles_)),
        game_index_(game_index) {
    CHECK(!players_.empty());
    CHECK_GE(initial_time, absl::ZeroDuration());
    CHECK_EQ(players_.size(), 2);
    positions_.reserve(30);
    racks_.reserve(15);
    bags_.reserve(30);
  }

  void CreateInitialPosition();

  void CreateInitialPosition(
      const Bag& ordered_bag,
      const std::vector<uint64_t>& exchange_insertion_dividends);

  const GamePosition* TwoPositionsAgo() const {
    if (positions_.size() < 2) {
      return nullptr;
    }
    return &(positions_[positions_.size() - 2]);
  }

  void AddNextPosition(const Move& move, absl::Duration time_elapsed);

  void Display(std::ostream& os) const;

  void FinishWithComputerPlayers();

  void AdjustGameEndScores();

  std::vector<int> Scores() const {
    std::vector<int> scores;
    scores.reserve(players_.size());
    if (positions_.size() % 2 == 0) {
      return {positions_.back().PlayerScore(),
              positions_.back().OpponentScore()};
    } else {
      return {positions_.back().OpponentScore(),
              positions_.back().PlayerScore()};
    }
  }

  const std::vector<GamePosition>& Positions() const { return positions_; }

  void WriteProto(q2::proto::GameResult* result) const;

 private:
  const BoardLayout& layout_;

  // In turn order.
  const std::vector<Player*>& players_;

  const Tiles& tiles_;

  const absl::Duration initial_time_;

  std::vector<GamePosition> positions_;

  // racks_[(2 * a) + b] is the rack for player b on turn a.
  std::vector<Rack> racks_;

  // Initialized with tiles_ but not ordered correctly until
  // CreateInitialPosition is called.
  Bag pregame_bag_;

  // Historical record of bag contents at each (half) turn.
  std::vector<Bag> bags_;

  absl::BitGen gen_;

  std::vector<uint64_t> exchange_insertion_dividends_;

  std::size_t exchange_dividend_index_;

  // Used for TileOrderingCache lookups
  const std::size_t game_index_;
};

#endif  // SRC_SCRABBLE_GAME_H