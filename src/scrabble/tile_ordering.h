#ifndef SRC_SCRABBLE_TILE_ORDERING_H
#define SRC_SCRABBLE_TILE_ORDERING_H

#include <cstdint>
#include <vector>

#include "absl/random/bit_gen_ref.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class TileOrdering {
 public:
  TileOrdering(const q2::proto::TileOrdering& config, const Tiles& tiles);
  TileOrdering(const std::vector<Letter>& letters,
               const std::vector<uint16_t>& exchange_insertion_dividends)
      : letters_(letters),
        exchange_insertion_dividends_(exchange_insertion_dividends) {}
  TileOrdering(const Tiles& tiles, absl::BitGenRef gen, int num_dividends);

  TileOrdering Adjust(const std::vector<Letter>& used_letters) const;

  const std::vector<Letter>& Letters() const { return letters_; }
  const std::vector<uint16_t>& ExchangeInsertionDividends() const {
    return exchange_insertion_dividends_;
  }

 private:
  const std::vector<Letter> letters_;
  const std::vector<uint16_t> exchange_insertion_dividends_;
};

#endif  // SRC_SCRABBLE_TILE_ORDERING_H