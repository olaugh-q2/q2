#include <iostream>
#include <iterator>
#include <string>

#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_layout.h"

int main(int argc, char *argv[]) {
  const auto layout =
      absl::make_unique<BoardLayout>("src/main/data/scrabble_board.textproto");
  const auto tiles = absl::make_unique<Tiles>(
      "src/main/data/english_scrabble_tiles.textproto");
  Board board;
  layout->DisplayBoard(board, *tiles, std::cout);
  return 0;
}