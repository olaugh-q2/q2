#include "src/scrabble/endgame_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move EndgamePlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  SetStartOfTurnTime();
  move_finders_[0]->FindMoves(pos.GetRack(), pos.GetBoard(),
                              pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                              true);
  const auto& on_moves_const = move_finders_[0]->Moves();
  // LOG(INFO) << "on_moves_const.size(): " << on_moves_const.size();
  std::vector<MoveWithDelta> on_moves;
  on_moves.reserve(on_moves_const.size());
  for (auto& move : on_moves_const) {
    on_moves.emplace_back(&move);
    on_moves.back().SetEquity(move.Score());
  }
  std::sort(on_moves.begin(), on_moves.end(),
            [](const MoveWithDelta& a, const MoveWithDelta& b) {
              return a.Score() > b.Score();
            });
  const GamePosition off_pos = pos.SwapRacks();
  move_finders_[1]->FindMoves(off_pos.GetRack(), off_pos.GetBoard(),
                              off_pos.GetUnseenToPlayer(),
                              MoveFinder::RecordAll, true);
  const auto& off_moves_const = move_finders_[1]->Moves();
  // LOG(INFO) << "off_moves_const.size(): " << off_moves_const.size();
  std::vector<MoveWithDelta> off_moves;
  off_moves.reserve(off_moves_const.size());
  for (auto& move : off_moves_const) {
    off_moves.emplace_back(&move);
    off_moves.back().SetEquity(move.Score());
  }
  std::sort(off_moves.begin(), off_moves.end(),
            [](const MoveWithDelta& a, const MoveWithDelta& b) {
              return a.Score() > b.Score();
            });
  auto on_moves_greedy = on_moves;
  for (auto& move : on_moves_greedy) {
    move.SetEquity(GreedyEndgameEquity(pos, move, on_moves, off_moves));
    // std::stringstream ss;
    // move.Display(tiles_, ss);
    // LOG(INFO) << "on_move: " << ss.str() << " " << move.Equity();
  }
  // LOG(INFO) << "Sorting " << on_moves_greedy.size() << " moves";
  MoveWithDelta* best_move = nullptr;
  for (auto& move : on_moves_greedy) {
    if (best_move == nullptr || move.Equity() > best_move->Equity()) {
      best_move = &move;
    }
  }
  /*
  std::sort(
      on_moves_greedy.begin(), on_moves_greedy.end(),
      [](const Move& a, const Move& b) { return a.Equity() > b.Equity(); });
  for (auto& move : on_moves_greedy) {
    std::stringstream ss;
    move.Display(tiles_, ss);
    LOG(INFO) << ss.str() << " " << move.Equity();
  }
  */
  // CHECK_GE(on_moves_greedy.size(), 1);
  // return on_moves_greedy[0];

  CHECK(best_move != nullptr);
  // std::stringstream ss;
  // best_move->Display(tiles_, ss);
  /// LOG(INFO) << "best_move: " << ss.str() << " " << best_move->Equity();
  return *best_move->GetMove();
}

float EndgamePlayer::GreedyEndgameEquity(const GamePosition& pos,
                                         const MoveWithDelta& move,
                                         std::vector<MoveWithDelta> on_moves,
                                         std::vector<MoveWithDelta> off_moves) {
  float net = move.Score();
  if (move.GetMove()->Leave().size() == 0) {
    // outplay
    int deadwood = 0;
    for (const Letter& letter : pos.GetUnseenToPlayer().Letters()) {
      deadwood += tiles_.Score(letter);
    }
    return net + deadwood * 2;
  }

  GamePosition pos_copy = pos;
  pos_copy.SetKnownOppRack(pos.GetUnseenToPlayer());
  const MoveWithDelta* move_to_place = &move;
  int scoreless = pos_copy.ScorelessTurns();
  std::vector<const Move*> moves_with_crosses_applied;
  for (int ply = 1;; ++ply) {
    //std::stringstream ss;
    //move_to_place->GetMove()->Display(tiles_, ss);
    //LOG(INFO) << "ply: " << ply << " move: " << ss.str();
    if (move_to_place->GetMove()->GetAction() == Move::Action::Place) {
      // LOG(INFO) << "placing move: " << ss.str();
      pos_copy.RemoveRackTiles(*move_to_place->GetMove());
      pos_copy.UnsafePlaceMove(*move_to_place->GetMove());
    }
    pos_copy.SwapWithKnownOppRack();

    const Board& board = pos_copy.GetBoard();
    //std::stringstream ss2;
    //pos_copy.Display(ss2);
    //LOG(INFO) << "pos: " << std::endl << ss2.str();
    int opp_deadwood = 0;
    for (const Letter& letter : pos_copy.GetKnownOppRack().Letters()) {
      opp_deadwood += tiles_.Score(letter);
    }
    bool on_turn = ply % 2 == 0;
    auto& moves = on_turn ? on_moves : off_moves;
    int sign = on_turn ? 1 : -1;
    // LOG(INFO) << "moves.size(): " << moves.size();
    // if (ply > 1) {
    //  RemoveMovesNotOnRack(&moves, new_pos.GetRack());
    //}
    // LOG(INFO) << "after removing moves with used tiles: moves.size(): "
    //           << moves.size();
    if (move_to_place->GetMove()->GetAction() == Move::Action::Place) {
      // move_finders_[0]->CacheCrossesAndScores(board);
      move_finders_[0]->CacheCrossesAndScores(board, *move_to_place->GetMove());
      moves_with_crosses_applied.push_back(move_to_place->GetMove());
    }
    // RemoveBlockedMoves(&moves, board);
    //  LOG(INFO) << "after removing blocked moves: moves.size(): " <<
    //  moves.size();
    const int size_of_outplays = pos_copy.GetRack().NumTiles();
    // AddDeadwood(&moves, opp_deadwood * 2, size_of_outplays);

    bool outplays_exist = false;
    for (auto& move : moves) {
      if (move.GetMove() == nullptr) {
        continue;
      }
      if (move.GetMove()->CachedNumTiles() == size_of_outplays) {
        outplays_exist = true;
        break;
      }
    }

    const auto& rack = pos_copy.GetRack();
    const int num_tiles = rack.NumTiles();
    const auto rack_counts = rack.Counts();
    uint32_t rack_bits = 0;
    for (const Letter letter : rack.Letters()) {
      rack_bits |= 1 << letter;
    }
    const uint32_t rack_mask = ~rack_bits;
    auto counts_copy = rack_counts;
    move_to_place = nullptr;
    for (auto& move : moves) {
      if (move.GetMove() == nullptr) {
        continue;
      }
      if (ply > 1) {
        /*
        std::stringstream ss;
        move.GetMove()->Display(tiles_, ss);
        LOG(INFO) << "move: " << ss.str();
        LOG(INFO) << "move.GetMove()->CachedNumTiles(): "
                  << move.GetMove()->CachedNumTiles();
        LOG(INFO) << "num_tiles: " << num_tiles;
        LOG(INFO) << "move.GetMove()->MoveBits(): "
                  << move.GetMove()->MoveBits();
        LOG(INFO) << "rack_mask: " << rack_mask;
        */
        if ((move.GetMove()->CachedNumTiles() > num_tiles) ||
            (move.GetMove()->MoveBits() & rack_mask)) {
          move.Nullify();
        } else {
          bool counts_copy_decremented = false;
          if (!move.GetMove()->IsSubsetOf(tiles_, &counts_copy,
                                          &counts_copy_decremented)) {
            move.Nullify();
          }
          if (counts_copy_decremented) {
            counts_copy = rack_counts;
          }
        }
        if (move.GetMove() == nullptr) {
          continue;
        }
      }
      if (move_finders_[0]->IsBlocked(*move.GetMove(), board)) {
        move.Nullify();
        continue;
      }
      // std::stringstream ss;
      // move.GetMove()->Display(tiles_, ss);
      float equity = move.Score();
      if (move.GetMove()->CachedNumTiles() == size_of_outplays) {
        equity = move.Score() + opp_deadwood * 2;
      }
      move.SetEquity(equity);
      // LOG(INFO) << "move: " << ss.str() << " " << move.Equity();
      if ((move_to_place == nullptr) ||
          (move.Equity() > move_to_place->Equity())) {
        move_to_place = &move;
        // If this is an outplay, or if there are no outplays, the remaining
        // plays are in descending order, so we can exit early.
        if (!outplays_exist ||
            move_to_place->GetMove()->CachedNumTiles() == size_of_outplays) {
          // LOG(INFO) << "that's an outplay, exit early";
          break;
        }
      }
    }
    // At very least, we can pass.
    CHECK_NE(move_to_place, nullptr);

    if (move_to_place->GetMove()->GetAction() == Move::Exchange) {
      scoreless++;
    } else {
      scoreless = 0;
    }
    if (scoreless >= 6) {
      int own_deadwood = 0;
      for (const Letter& letter : rack.Letters()) {
        own_deadwood += tiles_.Score(letter);
      }
      // LOG(INFO) << "opp_deadwood: " << opp_deadwood
      //           << " own_deadwood: " << own_deadwood;
      // LOG(INFO) << "returning " << net + sign * (opp_deadwood -
      // own_deadwood);
      for (int move_index = moves_with_crosses_applied.size() - 1;
           move_index >= 0; move_index--) {
        // std::stringstream ss3;
        // moves_with_crosses_applied[move_index]->Display(tiles_, ss3);
        // LOG(INFO) << "undoing move: " << ss3.str();
        pos_copy.UnsafeUndoMove(*moves_with_crosses_applied[move_index]);
        move_finders_[0]->CacheCrossesAndScores(
            board, *moves_with_crosses_applied[move_index]);
      }
      return net + sign * (opp_deadwood - own_deadwood);
    }
    int tiles_played = move_to_place->GetMove()->CachedNumTiles();
    net += sign * move_to_place->Score();
    // LOG(INFO) << "net: " << net;
    if (tiles_played == rack.NumTiles()) {
      // LOG(INFO) << "deadwood: " << opp_deadwood;
      // LOG(INFO) << "returning " << net + sign * opp_deadwood * 2;
      for (int move_index = moves_with_crosses_applied.size() - 1;
           move_index >= 0; move_index--) {
        pos_copy.UnsafeUndoMove(*moves_with_crosses_applied[move_index]);
        move_finders_[0]->CacheCrossesAndScores(
            board, *moves_with_crosses_applied[move_index]);
      }
      return net + sign * opp_deadwood * 2;
    }
  }
}

float EndgamePlayer::StaticEndgameEquity(const GamePosition& pos,
                                         const Move& move) const {
  if (move.GetAction() == Move::Action::Exchange) {
    return -9999;
  }
  float equity = move.Score();
  if (move.Leave().size() == 0) {
    // outplay
    int deadwood = 0;
    for (const Letter& letter : pos.GetUnseenToPlayer().Letters()) {
      deadwood += tiles_.Score(letter);
    }
    equity += deadwood * 2;
  }
  return equity;
}