#ifndef SRC_SCRABBLE_TILE_ORDERING_PROVIDER_H
#define SRC_SCRABBLE_TILE_ORDERING_PROVIDER_H

#include "src/scrabble/tile_ordering.h"

#include <vector>

class TileOrderingProvider {
 public:
  virtual ~TileOrderingProvider() = 0;
  virtual std::vector<TileOrdering> GetTileOrderings(int game_number,
                                                     int position_index,
                                                     int start_index,
                                                     int num_orderings) = 0;
};

#endif  // SRC_SCRABBLE_TILE_ORDERING_PROVIDER_H