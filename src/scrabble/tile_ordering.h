#ifndef SRC_SCRABBLE_TILE_ORDERING_H
#define SRC_SCRABBLE_TILE_ORDERING_H

#include <vector>

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

  TileOrdering Adjust(const std::vector<Letter>& used_letters) const;

  const std::vector<Letter>& GetLetters() const { return letters_; }
  const std::vector<uint16_t>& GetExchangeInsertionDividends() const {
    return exchange_insertion_dividends_;
  }

 private:
  const std::vector<Letter> letters_;
  const std::vector<uint16_t> exchange_insertion_dividends_;
};

#endif  // SRC_SCRABBLE_TILE_ORDERING_H