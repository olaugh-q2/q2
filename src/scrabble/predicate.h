#ifndef SRC_SCRABBLE_PREDICATE_H
#define SRC_SCRABBLE_PREDICATE_H

#include "src/scrabble/game_position.h"

class Predicate {
 public:
  virtual ~Predicate() = 0;
  virtual bool Evaluate(const GamePosition& position) const = 0;
};

#endif  // SRC_SCRABBLE_PREDICATE_H