#include "src/scrabble/alpha_beta_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move AlphaBetaPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  //LOG(INFO) << "AlphaBetaPlayer::ChooseBestMove";
  SetStartOfTurnTime();
  nodes_.clear();
  Move dummy;
  dummy.SetEquity(0.0);
  nodes_.emplace_back(absl::make_unique<GameNode>(dummy, nullptr, 0.0, 0));
  root_node_ = nodes_.back().get();
  GamePosition p = pos;
  p.SetKnownOppRack(pos.GetUnseenToPlayer());
  const GameNode* best_node;
  const GameState initial_state(
      tiles_.ToProduct64(p.GetRack().Letters()),
      tiles_.ToProduct64(p.GetKnownOppRack().Letters()),
      0,  // num_placed_letters
      {}  // initializes the array with zeroes
  );
  move_finder_->CacheCrossesAndScores(p.GetBoard());
  best_node =
      AlphaBeta(root_node_, initial_state, true, num_plies_, -1e10, 1e10, &p);
  // for (int i = 1; i <= num_plies_; ++i) {
  //   best_node = AlphaBeta(root_node_, initial_state, true, i, -1e10, 1e10,
  //   &p);
  // }
  CHECK_NOTNULL(best_node);
  if (best_node->GetMove().GetAction() == Move::Exchange) {
  } else if (best_node->GetMove().Score() <= 0) {
    std::stringstream ss;
    best_node->GetMove().Display(tiles_, ss);
    LOG(INFO) << "low best move: " << ss.str();
  } else if (best_node->GetMove().Score() >= 400) {
    std::stringstream ss;
    best_node->GetMove().Display(tiles_, ss);
    LOG(INFO) << "high best move: " << ss.str();
  }
  return best_node->GetMove();
}

std::vector<Move> AlphaBetaPlayer::MovesAfterParent(const GameNode* parent,
                                                    GamePosition* p) {
  const Bag empty_bag(tiles_, {});
  p->SwapWithKnownOppRack();
  move_finder_->FindMoves(p->GetRack(), p->GetBoard(), empty_bag,
                          MoveFinder::RecordAll, false);
  int opp_stuck_score = 0;
  const bool opp_stuck_with_tile =
      detect_stuck_tiles_ && move_finder_->StuckWithTile(&opp_stuck_score);
  if (opp_stuck_with_tile) {
    // LOG(INFO) << "opp stuck with tile, opp_stuck_score = " <<
    // opp_stuck_score; std::stringstream ss; p->Display(ss); LOG(INFO) <<
    // std::endl << ss.str();
  }
  p->SwapWithKnownOppRack();
  move_finder_->FindMoves(p->GetRack(), p->GetBoard(), empty_bag,
                          MoveFinder::RecordAll, false);
  // move_finder_->FindMoves(p->GetRack(), p->GetBoard(),
  // p->GetUnseenToPlayer(),
  //                         MoveFinder::RecordAll, false);
  bool preceded_by_pass = false;
  if (parent && (parent->GetMove().GetAction() != Move::Place)) {
    preceded_by_pass = true;
  }
  move_finder_->SetEndgameEquities(
      p->GetRack().Letters(), p->GetKnownOppRack().Letters(),
      opp_stuck_with_tile, opp_stuck_score, preceded_by_pass,
      stuck_tiles_left_multiplier_, stuck_leave_score_multiplier_,
      stuck_leave_value_multiplier_, opp_stuck_score_multiplier_,
      unstuck_leave_score_weight_, unstuck_leave_value_weight_, tiles_);
  move_finder_->SortMoves();
  return move_finder_->Moves();
}

AlphaBetaPlayer::GameNode* AlphaBetaPlayer::AlphaBeta(GameNode* parent,
                                                      const GameState& state,
                                                      bool on_turn, int depth,
                                                      float alpha, float beta,
                                                      GamePosition* p) {
  // std::stringstream ss;
  // p->Display(ss);
  // LOG(INFO) << "AlphaBetaPlayer::AlphaBeta: depth=" << depth
  //           << " on_turn=" << on_turn << " alpha=" << alpha << " beta=" << beta;
  // LOG(INFO) << "nodes_.size(): " << nodes_.size();
  // LOG(INFO) << "position: " << std::endl << ss.str();
  if (depth == 0) {
    parent->SetValue(parent->CalculateValue(*p, !on_turn, true));
    // LOG(INFO) << "depth = 0, return parent node with value " <<
    // parent->Value(); std::stringstream ss; parent->GetMove().Display(tiles_,
    // ss); LOG(INFO) << "parent move: " << ss.str();
    return parent;
  }
  if (p->GetKnownOppRack().NumTiles() == 0) {
    parent->SetValue(parent->CalculateValue(*p, !on_turn, true));
    // LOG(INFO) << "known opp rack is empty, there has been an outplay. return
    // "
    //              "parent node with value "
    //           << parent->Value();
    // std::stringstream ss;
    // parent->GetMove().Display(tiles_, ss);
    // LOG(INFO) << "parent move: " << ss.str();
    return parent;
  }
  const Rack original_rack = p->GetRack();
  GameNode* best_node = nullptr;
  const int ply = num_plies_ - depth;
  const int cap = CapAtPly(ply);
  //LOG(INFO) << "ply: " << ply << " cap: " << cap;
  int i = 0;
  std::vector<Move> moves = MovesAfterParent(parent, p);
  //LOG(INFO) << "moves.size(): " << moves.size();
  bool can_play = false;
  for (const auto& move : moves) {
    if (move.GetAction() == Move::Place) {
      can_play = true;
      break;
    }
  }
  if (on_turn) {
    float value = -1e10;
    for (const Move& move : moves) {
      std::stringstream ss;
      move.Display(tiles_, ss);
      // LOG(INFO) << "move #" << i << ": " << ss.str()
      //           << " equity: " << move.Equity();
      i++;
      if (i > cap) {
        // LOG(INFO) << "i > cap, breaking"
        //           << " i: " << i << " cap: " << cap;
        break;
      }
      if (can_play && (move.GetAction() != Move::Place)) {
        //LOG(INFO) << "Let's not pass since we have a scoring move";
        continue;
      }
      p->UnsafePlaceMove(move);
      // LOG(INFO) << "Placing move: " << ss.str();
      p->RemoveRackTiles(move);
      p->SwapWithKnownOppRack();
      move_finder_->CacheCrossesAndScores(p->GetBoard(), move);
      nodes_.emplace_back(
          absl::make_unique<GameNode>(move, parent, 0.0, depth));
      GameNode* child = nodes_.back().get();
      auto* best_child =
          AlphaBeta(child, state, !on_turn, depth - 1, alpha, beta, p);
      p->UnsafeUndoMove(move);
      // LOG(INFO) << "Undoing move: " << ss.str();
      p->SetKnownOppRack(original_rack);
      p->SwapWithKnownOppRack();
      move_finder_->CacheCrossesAndScores(p->GetBoard(), move);
      if (best_child->Value() > value) {
        value = best_child->Value();
        best_node = child;
      }
      if (value > alpha) {
        alpha = value;
      }
      if (alpha >= beta) {
        break;
      }
    }
    parent->SetValue(value);
    CHECK_NOTNULL(best_node);
    return best_node;
  } else {
    float value = +1e10;
    for (const Move& move : moves) {
      // std::stringstream ss;
      // move.Display(tiles_, ss);
      // LOG(INFO) << "move #" << i << ": " << ss.str()
      //           << " equity: " << move.Equity();
      i++;
      if (i > cap) {
        // LOG(INFO) << "i > cap, breaking"
        //           << " i: " << i << " cap: " << cap;
        break;
      }
      if (can_play && (move.GetAction() != Move::Place)) {
        // LOG(INFO) << "Let's not pass since we have a scoring move";
        continue;
      }
      p->UnsafePlaceMove(move);
      // LOG(INFO) << "Placing move: " << ss.str();
      p->RemoveRackTiles(move);
      p->SwapWithKnownOppRack();
      move_finder_->CacheCrossesAndScores(p->GetBoard(), move);
      nodes_.emplace_back(
          absl::make_unique<GameNode>(move, parent, 0.0, depth));
      GameNode* child = nodes_.back().get();
      auto* best_child =
          AlphaBeta(child, state, !on_turn, depth - 1, alpha, beta, p);
      p->UnsafeUndoMove(move);
      // LOG(INFO) << "Undoing move: " << ss.str();
      p->SetKnownOppRack(original_rack);
      p->SwapWithKnownOppRack();
      move_finder_->CacheCrossesAndScores(p->GetBoard(), move);
      if (best_child->Value() < value) {
        value = best_child->Value();
        best_node = child;
      }
      if (value < beta) {
        beta = value;
      }
      if (alpha >= beta) {
        break;
      }
    }
    parent->SetValue(value);
    CHECK_NOTNULL(best_node);
    return best_node;
  }
  return nullptr;
}