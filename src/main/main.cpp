#include <iostream>
#include <iterator>
#include <string>

#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/move.h"

int main(int argc, char* argv[]) {
  const auto layout =
      absl::make_unique<BoardLayout>("src/main/data/scrabble_board.textproto");
  const auto tiles = absl::make_unique<Tiles>(
      "src/main/data/english_scrabble_tiles.textproto");
  Board board;

  const auto& citizen = Move::Parse("8h CITIZEN", *tiles);
  board.UnsafePlaceMove(citizen.value());

  const auto& sh = Move::Parse("o8 SH", *tiles);
  board.UnsafePlaceMove(sh.value());

  const auto& shiplaps = Move::Parse("o8 ..IPLAPS", *tiles);
  board.UnsafePlaceMove(shiplaps.value());
  
  layout->DisplayBoard(board, *tiles, std::cout);
  return 0;
}