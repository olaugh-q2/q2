#include "src/scrabble/endgame_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move EndgamePlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  // LOG(INFO) << "EndgamePlayer::ChooseBestMove";
  SetStartOfTurnTime();
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordAll, true);
  const auto on_moves_const = move_finder_->Moves();
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
  move_finder_->FindMoves(off_pos.GetRack(), off_pos.GetBoard(),
                          off_pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                          true);
  const auto off_moves_const = move_finder_->Moves();
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
  float equity_to_beat = 9999;
  int i = 0;
  for (auto& move : on_moves_greedy) {
    if (++i > CapAtPly(0)) {
      break;
    }
    if (num_plies_ == 1) {
      if (move.GetMove()->Leave().size() > 0) {
        if (best_move != nullptr) {
          equity_to_beat = move.Score() - best_move->Equity();
          // std::stringstream ss;
          // best_move->GetMove()->Display(tiles_, ss);
          // LOG(INFO) << "best_move: " << ss.str() << " " <<
          // best_move->Equity(); std::stringstream ss2;
          // move.GetMove()->Display(tiles_, ss2);
          // LOG(INFO) << "move: " << ss2.str();
          // LOG(INFO) << "equity_to_beat: " << equity_to_beat;
        }
        move.SetEquity(EvaluateAllResponses(pos, equity_to_beat, move));
        // std::stringstream ss;
        // move.GetMove()->Display(tiles_, ss);
        // LOG(INFO) << "evaluated replies to move giving: " << ss.str() << " "
        //           << move.Equity();
      }
    }
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
  std::stringstream ss;
  best_move->GetMove()->Display(tiles_, ss);
  // LOG(INFO) << "best_move: " << ss.str() << " " << best_move->Equity();
  return *best_move->GetMove();
}

float EndgamePlayer::EvaluateAllResponses(GamePosition pos,
                                          float equity_to_beat,
                                          const MoveWithDelta& candidate_move) {
  // LOG(INFO) << "EvaluateAllResponses...";
  // std::stringstream move_ss;
  // candidate_move.GetMove()->Display(tiles_, move_ss);
  // LOG(INFO) << "candidate_move: " << move_ss.str();

  // std::stringstream pos_ss;
  // pos.Display(pos_ss);
  // LOG(INFO) << "pos: " << std::endl << pos_ss.str();
  pos.SetKnownOppRack(pos.GetUnseenToPlayer());
  pos.UnsafePlaceMove(*candidate_move.GetMove());
  pos.RemoveRackTiles(*candidate_move.GetMove());
  pos.SwapWithKnownOppRack();

  // std::stringstream pos_ss2;
  // pos.Display(pos_ss2);
  // LOG(INFO) << "pos: " << std::endl << pos_ss2.str();
  move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                          pos.GetUnseenToPlayer(), MoveFinder::RecordAll, true);
  const auto on_moves_const = move_finder_->Moves();
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
  move_finder_->FindMoves(off_pos.GetRack(), off_pos.GetBoard(),
                          off_pos.GetUnseenToPlayer(), MoveFinder::RecordAll,
                          true);
  const auto off_moves_const = move_finder_->Moves();
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
            int i = 0;
  auto on_moves_greedy = on_moves;
  for (auto& move : on_moves_greedy) {
    if (++i > CapAtPly(1)) {
      break;
    }
    move.SetEquity(GreedyEndgameEquity(pos, move, on_moves, off_moves));
    if (move.Equity() > (equity_to_beat + 1e-5)) {
      // std::stringstream ss;
      // move.GetMove()->Display(tiles_, ss);
      // LOG(INFO) << "returning early: " << ss.str() << " " << move.Equity()
      //           << " > " << equity_to_beat;
      return candidate_move.Score() - move.Equity();
    } else {
      // std::stringstream ss;
      // move.GetMove()->Display(tiles_, ss);
      // LOG(INFO) << "not returning early: " << ss.str() << " " <<
      // move.Equity()
      //           << " <= " << equity_to_beat;
    }
    // std::stringstream ss;
    // move.GetMove()->Display(tiles_, ss);
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
  // std::stringstream best_move_ss;
  // best_move->GetMove()->Display(tiles_, best_move_ss);
  // LOG(INFO) << "best_move: " << best_move_ss.str() << " "
  //           << best_move->Equity();
  return candidate_move.Score() - best_move->Equity();
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
    // std::stringstream ss;
    // move_to_place->GetMove()->Display(tiles_, ss);
    // LOG(INFO) << "ply: " << ply << " move: " << ss.str();
    if (move_to_place->GetMove()->GetAction() == Move::Action::Place) {
      // LOG(INFO) << "placing move: " << ss.str();
      pos_copy.RemoveRackTiles(*move_to_place->GetMove());
      pos_copy.UnsafePlaceMove(*move_to_place->GetMove());
    }
    pos_copy.SwapWithKnownOppRack();

    const Board& board = pos_copy.GetBoard();
    // std::stringstream ss2;
    // pos_copy.Display(ss2);
    // LOG(INFO) << "pos: " << std::endl << ss2.str();
    int opp_deadwood = 0;
    for (const Letter& letter : pos_copy.GetKnownOppRack().Letters()) {
      opp_deadwood += tiles_.Score(letter);
    }
    int own_deadwood = 0;
    const auto& rack = pos_copy.GetRack();
    for (const Letter& letter : rack.Letters()) {
      own_deadwood += tiles_.Score(letter);
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
      // move_finder_->CacheCrossesAndScores(board);
      move_finder_->CacheCrossesAndScores(board, *move_to_place->GetMove());
      moves_with_crosses_applied.push_back(move_to_place->GetMove());
    }
    // RemoveBlockedMoves(&moves, board);
    //  LOG(INFO) << "after removing blocked moves: moves.size(): " <<
    //  moves.size();
    const int size_of_outplays = pos_copy.GetRack().NumTiles();
    // AddDeadwood(&moves, opp_deadwood * 2, size_of_outplays);

    const int num_tiles = rack.NumTiles();
    const auto rack_counts = rack.Counts();
    uint32_t rack_bits = 0;
    for (const Letter letter : rack.Letters()) {
      rack_bits |= 1 << letter;
    }
    const uint32_t rack_mask = ~rack_bits;
    auto counts_copy = rack_counts;
    move_to_place = nullptr;
    float best_bonus = -9999;
    for (auto& move : moves) {
      if (move.GetMove() == nullptr) {
        continue;
      }
      if (ply > 1) {
        if ((move.GetMove()->CachedNumTiles() > num_tiles) ||
            (move.GetMove()->MoveBits() & rack_mask)) {
          move.Nullify();
          continue;
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
      float bonus = 0.0;
      if (move.GetMove()->CachedNumTiles() == size_of_outplays) {
        bonus = opp_deadwood * 2;
      } else if ((leave_score_weight_ > 1e-5) || (leave_value_weight_ > 1e-5)) {
        int leave_score = own_deadwood;
        auto leave = rack;
        LetterString move_tiles;
        for (const Letter& letter : move.GetMove()->Letters()) {
          if (letter == 0) {
            continue;
          }
          if (letter >= tiles_.BlankIndex()) {
            move_tiles.push_back(tiles_.BlankIndex());
            continue;
          }
          move_tiles.push_back(letter);
          leave_score -= tiles_.Score(letter);
        }

        // LOG(INFO) << "move_tiles: " << tiles_.ToString(move_tiles).value()
        //           << " leave: " << tiles_.ToString(leave.Letters()).value();

        bonus -= leave_score * leave_score_weight_;
        if (leave_value_weight_ > 1e-5) {
          if (leave.Letters().size() < 7) {
            leave.RemoveTiles(move_tiles, tiles_);
            float leave_value =
                leaves_.Value(tiles_.ToProduct64(leave.Letters()));
            bonus += leave_value * leave_value_weight_;
          }
        }
      }
      if (bonus > best_bonus) {
        best_bonus = bonus;
      }
      move.SetEquity(move.Score() + bonus);
    }
    // LOG(INFO) << "best_bonus: " << best_bonus;
    /*
    std::sort(moves.begin(), moves.end(),
              [](const MoveWithDelta& a, const MoveWithDelta& b) {
                if ((a.GetMove() == nullptr) && (b.GetMove() == nullptr)) {
                  return &a < &b;
                }
                if (a.GetMove() == nullptr) {
                  return false;
                }
                if (b.GetMove() == nullptr) {
                  return true;
                }
                return a.Equity() > b.Equity();
              });
    */
    for (auto& move : moves) {
      if (move.GetMove() == nullptr) {
        continue;
      }
      if ((move_to_place != nullptr) &&
          (move_to_place->Equity() > move.Score() + best_bonus)) {
        // Moves are in descending score order, so no remaining moves can reach
        // the equity of the best move.
        // std::stringstream ss;
        // move_to_place->GetMove()->Display(tiles_, ss);
        // LOG(INFO) << "move_to_place: " << ss.str() << " equity: "
        //          << move_to_place->Equity();
        // LOG(INFO) << "move scores are down to " << move.Score() << " so can't
        // reach best equity anymore";
        break;
      }
      if (move_finder_->IsBlocked(*move.GetMove(), board)) {
        move.Nullify();
        continue;
      }
      // std::stringstream ss;
      // move.GetMove()->Display(tiles_, ss);
      // LOG(INFO) << "move: " << ss.str() << " " << move.Equity();
      CHECK_NE(move.GetMove(), nullptr);
      if ((move_to_place == nullptr) ||
          (move.Equity() > move_to_place->Equity())) {
        move_to_place = &move;
      }
    }

    // At very least, we can pass.
    CHECK_NE(move_to_place, nullptr);

    if (move_to_place->GetMove()->GetAction() == Move::Exchange) {
      scoreless++;
    } else {
      scoreless = 0;
    }
    if (scoreless >= 2) {
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
        move_finder_->CacheCrossesAndScores(
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
        move_finder_->CacheCrossesAndScores(
            board, *moves_with_crosses_applied[move_index]);
      }
      // std::stringstream ss;
      // move_to_place->GetMove()->Display(tiles_, ss);
      // LOG(INFO) << "ply: " << ply + 1 << " move: " << ss.str();
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