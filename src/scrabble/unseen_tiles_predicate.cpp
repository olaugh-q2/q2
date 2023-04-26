#include "src/scrabble/unseen_tiles_predicate.h"

bool UnseenTilesPredicate::Evaluate(const GamePosition& position) const {
  const uint32_t num_unseen_tiles = position.GetUnseenToPlayer().Size();
  return num_unseen_tiles >= min_unseen_tiles_ &&
         num_unseen_tiles <= max_unseen_tiles_;
}
