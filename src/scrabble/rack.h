#ifndef SRC_SCRABBLE_RACK_H
#define SRC_SCRABBLE_RACK_H

#include "absl/container/flat_hash_map.h"
#include "src/scrabble/tiles.h"

class Rack {
 public:
  struct Products {
    absl::flat_hash_map<uint64_t, LetterString> subsets;
    absl::flat_hash_map<uint64_t, LetterString> blank_subsets;
    absl::flat_hash_map<uint64_t, LetterString> double_blank_subsets;
  };
  Rack(const LetterString& letters) : letters_(letters) {}
  const LetterString& Letters() const { return letters_; }
  int NumTiles() const { return letters_.size(); }
  int Capacity() const { return 7; }
  std::array<int, 32> Counts() const;
  absl::flat_hash_map<uint64_t, LetterString> Subsets(const Tiles& tiles) const;

  int NumBlanks(const Tiles& tiles) const;

  void Display(const Tiles& tiles, std::ostream& os) const;

  void PushBack(Letter letter) {
    CHECK(NumTiles() < Capacity());
    letters_.push_back(letter);
  }

  void RemoveTiles(const LetterString& letters, const Tiles& tiles);
  bool SafeRemoveTiles(const LetterString& letters, const Tiles& tiles);
  void Clear() { letters_.clear(); }

 private:
  LetterString letters_;
};

#endif  // SRC_SCRABBLE_RACK_H