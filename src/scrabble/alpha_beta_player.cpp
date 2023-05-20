#include "src/scrabble/alpha_beta_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move AlphaBetaPlayer::ChooseBestMove(
    const std::vector<GamePosition>* previous_positions,
    const GamePosition& pos) {
  // LOG(INFO) << "AlphaBetaPlayer::ChooseBestMove";
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
  best_node = AlphaBeta(root_node_, initial_state, true, 6, -1e10, 1e10, &p);
  // for (int i = 1; i <= num_plies_; ++i) {
  //   best_node = AlphaBeta(root_node_, initial_state, true, i, -1e10, 1e10,
  //   &p);
  // }
  CHECK_NOTNULL(best_node);
  // std::stringstream ss;
  // best_node->GetMove().Display(tiles_, ss);
  // LOG(INFO) << "best move: " << ss.str();
  return best_node->GetMove();
}

std::vector<Move> AlphaBetaPlayer::MovesAfterParent(const GameNode* parent,
                                                    GamePosition* p) {
  move_finder_->FindMoves(p->GetRack(), p->GetBoard(), p->GetUnseenToPlayer(),
                          MoveFinder::RecordAll, false);
  bool preceded_by_pass = false;
  if (parent && (parent->GetMove().GetAction() != Move::Place)) {
    preceded_by_pass = true;
  }
  move_finder_->SetEndgameEquities(p->GetRack().Letters(),
                                   p->GetKnownOppRack().Letters(),
                                   preceded_by_pass, tiles_);
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
  //           << " on_turn=" << on_turn << " alpha=" << alpha << " beta=" <<
  //           beta;
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
  int i = 0;
  if (on_turn) {
    float value = -1e10;
    std::vector<Move> moves = MovesAfterParent(parent, p);
    // LOG(INFO) << "moves.size(): " << moves.size();
    for (const Move& move : moves) {
      // std::stringstream ss;
      // move.Display(tiles_, ss);
      // LOG(INFO) << "move #" << i++ << ": " << ss.str();
      i++;
      if (depth >= 5) {
        if (i > 20) {
          break;
        }
      } else if (depth >= 3) {
        if (i > 4) {
          break;
        }
      } else {
        if (i > 2) {
          break;
        }
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
    std::vector<Move> moves = MovesAfterParent(parent, p);
    // LOG(INFO) << "moves.size(): " << moves.size();
    for (const Move& move : moves) {
      // std::stringstream ss;
      // move.Display(tiles_, ss);
      //   LOG(INFO) << "move #" << i++ << ": " << ss.str();
      i++;
      if (depth >= 5) {
        if (i > 4) {
          break;
        }
      } else if (depth >= 3) {
        if (i > 4) {
          break;
        }
      } else {
        if (i > 2) {
          break;
        }
      }
      p->UnsafePlaceMove(move);
      // LOG(INFO) << "Placing move: " << ss.str();
      p->RemoveRackTiles(move);
      p->SwapWithKnownOppRack();
      move_finder_->CacheCrossesAndScores(p->GetBoard(), move);
      GameNode* child = new GameNode(move, parent, 0.0, depth);
      nodes_.emplace_back(child);
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