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
  positions_.emplace_back(layout_, board, players_[0].Id(), players_[1].Id(), player_rack, 0, 0, 0,
                          initial_time_, 0, tiles_);
}

void Game::AddNextPosition(const Move& move, absl::Duration time_elapsed) {
  CHECK(!positions_.empty());
  positions_.back().CommitMove(move, time_elapsed);
  Board board = positions_.back().GetBoard();
  if (move.Action() == Move::Place) {
    board.UnsafePlaceMove(move);
  }
  CHECK_GE(racks_.size(), 2);
  Rack rack = racks_[racks_.size() - 2];
  std::stringstream ss;
  rack.Display(tiles_, ss);
  LOG(INFO) << "rack (before removal): " << ss.str();
  LOG(INFO) << "rack.Size() = " << rack.NumTiles();
  std::stringstream ss_move;
  move.Display(tiles_, ss_move);
  LOG(INFO) << "move: " << ss_move.str();
  rack.RemoveTiles(move.Letters());
  LOG(INFO) << "rack.Size() = " << rack.NumTiles();
  Bag bag = bags_.back();
  std::stringstream ss1;
  bag.Display(ss1);
  LOG(INFO) << "bag (before completion): " << ss1.str();
  bag.CompleteRack(&rack);
  std::stringstream ss2;
  bag.Display(ss2);
  LOG(INFO) << "bag (after completion): " << ss2.str();
  if (move.Action() == Move::Exchange) {
    bag.InsertTiles(move.Letters(), exchange_insertion_dividends_,
                    &exchange_dividend_index_);
  }
  std::stringstream ss3;
  bag.Display(ss3);
  LOG(INFO) << "bag (after insertion): " << ss3.str();

  absl::Duration time_remaining = initial_time_;
  if (const GamePosition* two_positions_ago = TwoPositionsAgo()) {
    time_remaining = two_positions_ago->TimeRemainingEnd();
  }

  int scoreless_turns = 0;
  if (positions_.back().IsScorelessTurn()) {
    scoreless_turns = positions_.back().ScorelessTurns() + 1;
  }
  positions_.emplace_back(layout_, board, positions_.back().OpponentPlayerId(),
                          positions_.back().OnTurnPlayerId(), racks_.back(),
                          positions_.back().OpponentScore(),
                          positions_.back().PlayerScore() + move.Score(),
                          positions_.back().PositionIndex() + 1, time_remaining,
                          scoreless_turns,
                          tiles_);
  bags_.emplace_back(bag);
  LOG(INFO) << "Bags:";
  for (const auto& bag : bags_) {
    std::stringstream ss;
    bag.Display(ss);
    LOG(INFO) << ss.str();
  }
  racks_.emplace_back(rack);
  LOG(INFO) << "Racks:";
  for (const auto& rack : racks_) {
    std::stringstream ss;
    rack.Display(tiles_, ss);
    LOG(INFO) << ss.str();
  }
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
        player_idx++;
        if (player_idx >= players_.size()) {
          player_idx = 0;
        }
      }
      os << std::endl;
    }
    os << "Current position: " << std::endl;
    positions_.back().Display(os, pregame_bag_);
  }
}