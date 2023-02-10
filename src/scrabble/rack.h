#ifndef SRC_SCRABBLE_RACK_H
#define SRC_SCRABBLE_RACK_H

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "src/scrabble/tiles.h"

class Rack {
 public:
  struct Products {
    absl::flat_hash_map<absl::uint128, LetterString> subsets;
    absl::flat_hash_map<absl::uint128, LetterString> blank_subsets;
    absl::flat_hash_map<absl::uint128, LetterString> double_blank_subsets;
  };
  Rack(const LetterString& letters) : letters_(letters) {}
  int NumTiles() const { return letters_.size(); }
  std::array<int, 32> Counts() const;
  absl::flat_hash_map<absl::uint128, LetterString> Subsets(const Tiles& tiles) const;
  
  int NumBlanks(const Tiles& tiles) const;

  void Display(const Tiles& tiles, std::ostream& os) const;

 private:
  LetterString letters_;
};

#endif  // SRC_SCRABBLE_RACK_H