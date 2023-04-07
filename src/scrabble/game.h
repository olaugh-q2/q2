#ifndef SRC_SCRABBLE_GAME_H
#define SRC_SCRABBLE_GAME_H

#include "absl/random/random.h"
#include "absl/time/time.h"
#include "src/scrabble/bag.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/player.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

class Game {
 public:
  Game(const BoardLayout& layout, const std::vector<Player>& players,
       const Tiles& tiles, absl::Duration initial_time)
      : layout_(layout),
        players_(players),
        tiles_(tiles),
        initial_time_(initial_time) {
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

  void Display(std::ostream& os) const;

 private:
  const BoardLayout& layout_;

  // In turn order.
  const std::vector<Player>& players_;

  const Tiles& tiles_;

  const absl::Duration initial_time_;

  std::vector<GamePosition> positions_;

  // racks_[(2 * a) + b] is the rack for player b on turn a.
  std::vector<Rack> racks_;

  // Historical record of bag contents at each (half) turn.
  std::vector<Bag> bags_;

  absl::BitGen gen_;

  std::vector<uint64_t> exchange_insertion_dividends_;

  int exchange_dividend_index_;
};

#endif  // SRC_SCRABBLE_GAME_H