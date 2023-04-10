#ifndef SRC_SCRABBLE_BAG_H
#define SRC_SCRABBLE_BAG_H

#include "absl/random/bit_gen_ref.h"
#include "src/scrabble/board.h"
#include "src/scrabble/rack.h"
#include "src/scrabble/tiles.h"

class Bag {
 public:
  explicit Bag(const Tiles& tiles);
  Bag(const Bag&) = default;

  Bag(const Tiles& tiles, const std::vector<Letter>& letters)
      : tiles_(tiles), letters_(letters) {}
  Bag UnseenToPlayer(const Board& board, const Rack& rack) const;
  int Size() const { return letters_.size(); }
  bool CanExchange() const { return Size() >= 7; }
  void Display(std::ostream& os) const;
  void Shuffle(absl::BitGenRef gen) {
    std::shuffle(letters_.begin(), letters_.end(), gen);
  }
  void CompleteRack(Rack* rack);
  const std::vector<Letter>& Letters() const { return letters_; }
  void SetLetters(const std::vector<Letter>& letters) { letters_ = letters; }
  void InsertTiles(const LetterString& tiles,
                   const std::vector<uint64_t>& exchange_insertion_dividends,
                   std::size_t* exchange_insertion_index);

 private:
  const Tiles& tiles_;
  std::vector<Letter> letters_;
};

#endif  // SRC_SCRABBLE_BAG_H