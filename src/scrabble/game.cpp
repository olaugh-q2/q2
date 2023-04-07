#include "src/scrabble/game.h"

#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/player.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

void Game::CreateInitialPosition() {
  Bag bag(tiles_);
  bags_.emplace_back(bag);
  bag.Shuffle(gen_);
  std::vector<uint64_t> exchange_insertion_dividends;
  for (int i = 0; i < 1000; ++i) {
    exchange_insertion_dividends.push_back(absl::Uniform<uint64_t>(gen_));
  }
  CreateInitialPosition(bag, exchange_insertion_dividends);
}

void Game::CreateInitialPosition(
    const Bag& ordered_bag,
    const std::vector<uint64_t>& exchange_insertion_dividends) {
  Board board;
  exchange_dividend_index_ = 0;
  exchange_insertion_dividends_ = exchange_insertion_dividends;
  pregame_bag_.SetLetters(ordered_bag.Letters());
  LetterString empty;
  Rack player_rack(empty);
  Rack opponent_rack(empty);
  Bag new_bag = ordered_bag;
  new_bag.CompleteRack(&player_rack);
  new_bag.CompleteRack(&opponent_rack);
  racks_.emplace_back(player_rack);
  racks_.emplace_back(opponent_rack);
    bags_.emplace_back(new_bag);
  positions_.emplace_back(layout_, board, 0, 1, player_rack, 0, 0, nullptr,
                          initial_time_, tiles_);
}

void Game::Display(std::ostream& os) const {
  for (const Player& player : players_) {
    player.Display(os);
    os << std::endl;
  }
  os << std::endl;
  if (positions_.empty()) {
    os << "No positions." << std::endl;
  } else {
    if (positions_.size() > 1) {
      os << "Game log: " << std::endl;
      int player_idx = 0;
      for (int i = 0; i < positions_.size() - 1; ++i) {
        os << "  " << players_[player_idx].Nickname() << " played: ";
        const auto move = positions_[i].GetMove();
        CHECK(move.has_value());
        move->Display(tiles_, os);
        os << std::endl;
      }
    }
    os << "Current position: " << std::endl;
    positions_.back().Display(os);
  }
}