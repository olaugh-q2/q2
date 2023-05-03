#include "src/scrabble/endgame_player.h"

#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

Move EndgamePlayer::ChooseBestMove(const GamePosition& pos) {
  SetStartOfTurnTime();
  auto on_moves =
      move_finder_->FindMoves(pos.GetRack(), pos.GetBoard(),
                              pos.GetUnseenToPlayer(), MoveFinder::RecordAll);
  for (auto& move : on_moves) {
    move.SetEquity(StaticEndgameEquity(pos, move));
  }
  const GamePosition off_pos = pos.SwapRacks();
  auto off_moves = move_finder_->FindMoves(
      off_pos.GetRack(), off_pos.GetBoard(), off_pos.GetUnseenToPlayer(),
      MoveFinder::RecordAll);
  for (auto& move : off_moves) {
    move.SetEquity(StaticEndgameEquity(pos, move));
    // std::stringstream ss;
    // move.Display(tiles_, ss);
    // LOG(INFO) << "off_move: " << ss.str() << " " << move.Equity();
  }
  // std::sort(on_moves.begin(), on_moves.end(), [](const Move& a, const Move&
  // b) {
  //   return a.Equity() > b.Equity();
  // });
  auto on_moves_greedy = on_moves;
  for (auto& move : on_moves_greedy) {
    move.SetEquity(GreedyEndgameEquity(pos, move, on_moves, off_moves));
    // std::stringstream ss;
    // move.Display(tiles_, ss);
    // LOG(INFO) << "on_move: " << ss.str() << " " << move.Equity();
  }
  // LOG(INFO) << "Sorting " << on_moves_greedy.size() << " moves";
  Move* best_move = nullptr;
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
  //std::stringstream ss;
  //best_move->Display(tiles_, ss);
  ///LOG(INFO) << "best_move: " << ss.str() << " " << best_move->Equity();
  return *best_move;
}

void EndgamePlayer::RemoveMovesNotOnRack(std::vector<Move>* moves,
                                         const Rack& rack) {
  auto it = std::remove_if(moves->begin(), moves->end(),
                           [this, &rack](const Move& move) {
                             return !move.IsSubsetOf(tiles_, rack);
                           });
  moves->erase(it, moves->end());
}

void EndgamePlayer::RemoveBlockedMoves(std::vector<Move>* moves,
                                       const Board& board) {
  auto it = std::remove_if(moves->begin(), moves->end(),
                           [this, board](const Move& move) {
                             return move_finder_->IsBlocked(move, board);
                           });
  moves->erase(it, moves->end());
}

namespace {
void AddDeadwood(std::vector<Move>* moves, int deadwood_points, int num_tiles) {
  for (auto& move : *moves) {
    float equity = move.Score();
    CHECK_LE(move.NumTiles(), num_tiles);
    if (move.NumTiles() == num_tiles) {
      equity += deadwood_points;
    }
    move.SetEquity(equity);
  }
}
}  // namespace
float EndgamePlayer::GreedyEndgameEquity(const GamePosition& pos,
                                         const Move& move,
                                         std::vector<Move> on_moves,
                                         std::vector<Move> off_moves) {
  float net = move.Score();
  if (move.Leave().size() == 0) {
    // outplay
    int deadwood = 0;
    for (const Letter& letter : pos.GetUnseenToPlayer().Letters()) {
      deadwood += tiles_.Score(letter);
    }
    return net + deadwood * 2;
  }

  std::vector<GamePosition> positions({pos});
  const Move* move_to_place = &move;
  int scoreless = pos.ScorelessTurns();
  for (int ply = 1;; ++ply) {
    // std::stringstream ss;
    // move_to_place->Display(tiles_, ss);
    // LOG(INFO) << "ply: " << ply << " move: " << ss.str();
    positions.push_back(positions.back().SwapRacks());
    std::stringstream ss2;
    GamePosition& new_pos = positions.back();
    Board board = new_pos.GetBoard();
    if (move_to_place->GetAction() == Move::Action::Place) {
      board.UnsafePlaceMove(*move_to_place);
    }
    new_pos.SetBoard(board);
    // positions.back().Display(ss2);
    // LOG(INFO) << "pos: " << std::endl << ss2.str();
    int opp_deadwood = 0;
    auto unseen = new_pos.GetUnseenToPlayer();
    for (const Letter& letter : unseen.Letters()) {
      opp_deadwood += tiles_.Score(letter);
    }
    bool on_turn = ply % 2 == 0;
    auto& moves = on_turn ? on_moves : off_moves;
    int sign = on_turn ? 1 : -1;
    // LOG(INFO) << "moves.size(): " << moves.size();
    RemoveMovesNotOnRack(&moves, new_pos.GetRack());
    // LOG(INFO) << "after removing moves with used tiles: moves.size(): "
    //           << moves.size();
    RemoveBlockedMoves(&moves, board);
    // LOG(INFO) << "after removing blocked moves: moves.size(): " <<
    // moves.size();
    AddDeadwood(&moves, opp_deadwood * 2, new_pos.GetRack().Letters().size());

    move_to_place = nullptr;
    for (auto& move : moves) {
      // std::stringstream ss;
      // move.Display(tiles_, ss);
      // LOG(INFO) << "move: " << ss.str() << " " << move.Equity();
      if ((move_to_place == nullptr) ||
          (move.Equity() > move_to_place->Equity())) {
        move_to_place = &move;
      }
    }
    // At very least, we can pass.
    CHECK_NE(move_to_place, nullptr);

    if (move_to_place->GetAction() == Move::Exchange) {
      scoreless++;
    } else {
      scoreless = 0;
    }
    if (scoreless >= 6) {
      int own_deadwood = 0;
      for (const Letter& letter : new_pos.GetRack().Letters()) {
        own_deadwood += tiles_.Score(letter);
      }
      // LOG(INFO) << "opp_deadwood: " << opp_deadwood
      //           << " own_deadwood: " << own_deadwood;
      // LOG(INFO) << "returning " << net + sign * (opp_deadwood -
      // own_deadwood);
      return net + sign * (opp_deadwood - own_deadwood);
    }
    int tiles_played = 0;
    for (const auto& letter : move_to_place->Letters()) {
      if (letter > 0) {
        ++tiles_played;
      }
    }
    net += sign * move_to_place->Score();
    // LOG(INFO) << "net: " << net;
    if (tiles_played == new_pos.GetRack().NumTiles()) {
      // LOG(INFO) << "deadwood: " << opp_deadwood;
      // LOG(INFO) << "returning " << net + sign * opp_deadwood * 2;
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