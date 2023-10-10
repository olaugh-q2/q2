#include "src/scrabble/simming_player.h"

#include "src/scrabble/data_manager.h"
#include "src/scrabble/game.h"
#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move SimmingPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  SetStartOfTurnTime();
  std::stringstream ss;
  pos.Display(ss);
  LOG(INFO) << "SimmingPlayer::ChooseBestMove: " << std::endl << ss.str();
  auto all_moves = FindMoves(previous_positions, pos);
  std::sort(
      all_moves.begin(), all_moves.end(),
      [](const Move& m1, const Move& m2) { return m1.Equity() > m2.Equity(); });
  auto candidates = InitialPrune(all_moves);
  auto* cf = ComponentFactory::GetInstance();
  auto* ordering_provider = cf->GetTileOrderingProvider();
  auto orderings = ordering_provider->GetTileOrderings(
      pos.GameIndex(), pos.PositionIndex(), 0, min_iterations_);
  /*
  for (const auto& ordering : orderings) {
    std::string letters;
    for (const auto& letter : ordering.Letters()) {
      letters += tiles_.NumberToChar(letter).value();
    }
    LOG(INFO) << "ordering: " << letters;
  }
  */
  const auto seen = pos.SeenByPlayer();
  std::vector<TileOrdering> adjusted_orderings;
  adjusted_orderings.reserve(orderings.size());
  for (const auto& ordering : orderings) {
    adjusted_orderings.push_back(ordering.Adjust(seen));
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const MoveWithResults& m1, const MoveWithResults& m2) {
              return m1.GetMove()->Equity() > m2.GetMove()->Equity();
            });
  for (auto& move : candidates) {
    std::stringstream ss;
    move.GetMove()->Display(tiles_, ss);
    LOG(INFO) << "candidate move: " << ss.str() << " equity: " << move.GetMove()->Equity();
  }
  if (candidates.size() == 1) {
    std::stringstream ss;
    candidates[0].GetMove()->Display(tiles_, ss);
    LOG(INFO) << "returning sole candidate " << ss.str();
    return *candidates[0].GetMove();
  }
  SimMoves(pos, adjusted_orderings, &candidates);
  const MoveWithResults* best_move = nullptr;
  std::sort(candidates.begin(), candidates.end(),
            [](const MoveWithResults& m1, const MoveWithResults& m2) {
              return m1.AverageSpread() > m2.AverageSpread();
            });
  for (auto& move : candidates) {
    std::stringstream ss;
    move.GetMove()->Display(tiles_, ss);
    LOG(INFO) << "simmed move: " << ss.str() << " spread: " << move.AverageSpread();
    if (best_move == nullptr ||
        move.AverageSpread() > best_move->AverageSpread()) {
      best_move = &move;
    }
  }
  CHECK_NOTNULL(best_move);
  return *best_move->GetMove();
}

std::vector<Move> SimmingPlayer::FindMoves(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  std::stringstream ss;
  pos.Display(ss);
  //LOG(INFO) << "SimmingPlayer::FindMoves: " << std::endl << ss.str();
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                          true);
  return move_finder_->Moves();
}

std::vector<SimmingPlayer::MoveWithResults> SimmingPlayer::InitialPrune(
    const std::vector<Move>& moves) {
  std::vector<SimmingPlayer::MoveWithResults> ret;
  ret.reserve(moves.size());
  for (const auto& move : moves) {
    ret.emplace_back(&move);
  }
  const size_t new_size =
      std::min(ret.size(), static_cast<size_t>(max_plays_considered_));
  std::partial_sort(ret.begin(), ret.begin() + new_size, ret.end(),
                    [](const MoveWithResults& a, const MoveWithResults& b) {
                      return a.GetMove()->Equity() > b.GetMove()->Equity();
                    });

  CHECK_GE(ret.size(), 1);
  const float best_equity = ret[0].GetMove()->Equity();
  // Find the first move that should be pruned.
  auto it = std::find_if(ret.begin(), ret.end(),
                         [this, best_equity](const MoveWithResults& a) {
                           return (best_equity - a.GetMove()->Equity()) >
                                  static_equity_pruning_threshold_;
                         });

  // Erase that move and all moves after it.
  if (it != ret.end()) {
    ret.erase(it, ret.end());
  }

  // Resize to the number of max plays considered, if needed.
  ret.resize(std::min(ret.size(), new_size));
  return ret;
}

void SimmingPlayer::RecordResults(const Game& game,
                                  MoveWithResults* move) const {
  float spread = 0.0;
  for (int i = 0; i <= num_plies_; ++i) {
    int sign = (i % 2 == 0) ? 1 : -1;
    const auto* pos = game.GetPosition(i);
    CHECK(pos != nullptr);
    const auto& move = pos->GetMove();
    CHECK(move.has_value());
    if (i + 2 > num_plies_) {
      spread += sign * move->Equity();
    } else {
      spread += sign * move->Score();
    }
  }
  move->RecordSpread(spread);
}

void SimmingPlayer::SimMove(const GamePosition& position,
                            const std::vector<TileOrdering>& orderings,
                            MoveWithResults* move) const {
  std::vector<Player*> players;
  for (int i = 0; i < 2; i++) {
    auto& player = rollout_players_[i];
    player->ResetGameState();
    players.push_back(player.get());
  }
  move->IncrementIterations(orderings.size());
  for (const auto& ordering : orderings) {
    Game game(layout_, position, players, tiles_, ordering);
    game.AddNextPosition(*move->GetMove(), absl::ZeroDuration());
    game.ContinueWithComputerPlayers(num_plies_);
    //std::stringstream ss;
    //game.Display(ss);
    //LOG(INFO) << "game: " << std::endl << ss.str();
    RecordResults(game, move);
  }
}

void SimmingPlayer::SimMoves(const GamePosition& position,
                             const std::vector<TileOrdering>& orderings,
                             std::vector<MoveWithResults>* moves) const {
  for (auto& move : *moves) {
    SimMove(position, orderings, &move);
  }
}