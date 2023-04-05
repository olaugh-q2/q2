#include "src/scrabble/game_position.h"

void GamePosition::Display(std::ostream& os) const {
  layout_.DisplayBoard(board_, tiles_, os);
}