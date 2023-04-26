#include "src/scrabble/game.h"

#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/player.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

void Game::CreateInitialPosition() {
  Bag bag(tiles_);
  bag.Shuffle(gen_);
  bags_.emplace_back(bag);
  std::vector<uint64_t> exchange_insertion_dividends;
  for (int i = 0; i < 1000; ++i) {
    exchange_insertion_dividends.push_back(absl::Uniform<uint64_t>(gen_));
  }
  CreateInitialPosition(bag, exchange_insertion_dividends);
}

void Game::CreateInitialPosition(
    const Bag& ordered_bag,
    const std::vector<uint64_t>& exchange_insertion_dividends) {
  //LOG(INFO) << "CreateInitialPosition";
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
  positions_.emplace_back(layout_, board, players_[0]->Id(), players_[1]->Id(),
                          player_rack, 0, 0, 0, initial_time_, 0, tiles_);
}

void Game::AddNextPosition(const Move& move, absl::Duration time_elapsed) {
  //LOG(INFO) << "AddNextPosition(...) elapsed: " << time_elapsed;
  CHECK(!positions_.empty());
  positions_.back().CommitMove(move, time_elapsed);
  Board board = positions_.back().GetBoard();
  if (move.GetAction() == Move::Place) {
    board.UnsafePlaceMove(move);
  }
  CHECK_GE(racks_.size(), 2);
  Rack rack = racks_[racks_.size() - 2];
  std::stringstream ss;
  rack.Display(tiles_, ss);
  //LOG(INFO) << "rack (before removal): " << ss.str();
  //LOG(INFO) << "rack.Size() = " << rack.NumTiles();
  std::stringstream ss_move;
  move.Display(tiles_, ss_move);
  //LOG(INFO) << "move: " << ss_move.str();
  LetterString move_tiles = move.Letters();
  for (auto& tile : move_tiles) {
    if (tile >= tiles_.BlankIndex()) {
      tile = tiles_.BlankIndex();
    }
  }
  int scoreless_turns = 0;
  Bag bag = bags_.back();
  absl::Duration time_remaining = initial_time_;
  if (const GamePosition* two_positions_ago = TwoPositionsAgo()) {
    time_remaining = two_positions_ago->TimeRemainingEnd();
  }
  // No bag manipulation is needed for deadwood.
  if (move.GetAction() == Move::Exchange || move.GetAction() == Move::Place) {
    move_tiles = move.Letters();
    rack.RemoveTiles(move_tiles, tiles_);
    //LOG(INFO) << "rack.Size() = " << rack.NumTiles();
    std::stringstream ss1;
    bag.Display(ss1);
    //LOG(INFO) << "bag (before completion): " << ss1.str();
    bag.CompleteRack(&rack);
    std::stringstream ss2;
    bag.Display(ss2);
    //LOG(INFO) << "bag (after completion): " << ss2.str();
    if (move.GetAction() == Move::Exchange) {
      bag.InsertTiles(move.Letters(), exchange_insertion_dividends_,
                      &exchange_dividend_index_);
    }
    std::stringstream ss3;
    bag.Display(ss3);
    //LOG(INFO) << "bag (after insertion): " << ss3.str();

    if (positions_.back().IsScorelessTurn()) {
      scoreless_turns = positions_.back().ScorelessTurns() + 1;
    }
  }
  positions_.emplace_back(layout_, board, positions_.back().OpponentPlayerId(),
                          positions_.back().OnTurnPlayerId(), racks_.back(),
                          positions_.back().OpponentScore(),
                          positions_.back().PlayerScore() + move.Score(),
                          positions_.back().PositionIndex() + 1, time_remaining,
                          scoreless_turns, tiles_);
  bags_.emplace_back(bag);
  /*
  LOG(INFO) << "Bags:";
  for (const auto& bag : bags_) {
    std::stringstream ss;
    bag.Display(ss);
    LOG(INFO) << ss.str();
  }
  */
  racks_.emplace_back(rack);
  /*
  LOG(INFO) << "Racks:";
  for (const auto& rack : racks_) {
    std::stringstream ss;
    rack.Display(tiles_, ss);
    LOG(INFO) << ss.str();
  }
  */
}

void Game::Display(std::ostream& os) const {
  for (const Player* player : players_) {
    player->Display(os);
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
        os << "  " << players_[player_idx]->Nickname() << " played: ";
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

void Game::AdjustGameEndScores() {
  //LOG(INFO) << "AdjustGameEndScores()";
  // Assumed that this is only called when the game is over.
  CHECK(positions_.back().IsGameOver());
  // This only makes sense for 2 player games.
  CHECK(players_.size() == 2);
  if (positions_.back().ScorelessTurns() >= 6) {
    for (int i = 0; i < 2; i++) {
      const LetterString& letters = positions_.back().GetRack().Letters();
      int penalty = -1 * tiles_.Score(letters);
      Move move(Move::OwnDeadwoodPenalty, letters, penalty);
      AddNextPosition(move, absl::ZeroDuration());
    }
  } else {
    // The "on-turn" player is the one stuck with letters. We commit a pass for
    // them and give their tiles for the bonus to the other player, who must
    // have played out.
    const LetterString& letters = positions_.back().GetRack().Letters();
    const LetterString empty;
    Move pass(Move::Exchange, empty, 0);
    AddNextPosition(pass, absl::ZeroDuration());
    int bonus = 2 * tiles_.Score(letters);
    Move move(Move::OppDeadwoodBonus, letters, bonus);
    AddNextPosition(move, absl::ZeroDuration());
  }
}

void Game::FinishWithComputerPlayers() {
  //LOG(INFO) << "FinishWithComputerPlayers()";
  CHECK(!positions_.empty());
  CHECK(players_.size() == 2);
  CHECK(players_[0]->GetPlayerType() == Player::Computer);
  CHECK(players_[1]->GetPlayerType() == Player::Computer);
  while (!positions_.back().IsGameOver()) {
    const int player_index = positions_.back().PositionIndex() % 2;
    //LOG(INFO) << "Player index: " << player_index;
    Player* player_ptr = players_[player_index];
    ComputerPlayer* computer_player = dynamic_cast<ComputerPlayer*>(player_ptr);
    //LOG(INFO) << "Player: " << computer_player->Id();
    const auto start_time = absl::Now();
    //LOG(INFO) << "start_time: " << start_time;
    std::stringstream ss;
    positions_.back().Display(ss);
    computer_player->Display(ss);
    //LOG(INFO) << "positions_.back(): " << std::endl << ss.str() << std::endl;
    const auto move = computer_player->ChooseBestMove(positions_.back());
    AddNextPosition(move, absl::Now() - start_time);
  }
  AdjustGameEndScores();
}

void Game::WriteProto(q2::proto::GameResult* result) const {
  for (const Player* player : players_) {
    result->add_player_ids(player->Id());
    if (positions_.back().OnTurnPlayerId() == player->Id()) {
      result->add_player_scores(positions_.back().PlayerScore());
    } else {
      result->add_player_scores(positions_.back().OpponentScore());
    }
  }
  for (const auto& rack : racks_) {
    result->add_racks(tiles_.ToString(rack.Letters()).value());
  }
  for (const auto& bag : bags_) {
    result->add_bags(bag.ToString());
  }
  for (const auto& position : positions_) {
    position.WriteProto(result->add_game_positions());
  }
}