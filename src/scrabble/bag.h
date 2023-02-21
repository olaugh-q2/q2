#ifndef SRC_SCRABBLE_BAG_H_
#define SRC_SCRABBLE_BAG_H_

#include "src/scrabble/tiles.h"

class Bag {
 public:
  explicit Bag(const Tiles& tiles);
  Bag(const Tiles& tiles, const std::vector<Letter>& letters)
      : tiles_(tiles), letters_(letters) {}
  int Size() const { return letters_.size(); }
  bool CanExchange() const { return Size() >= 7; }
  void Display(std::ostream& os) const;

 private:
  const Tiles& tiles_;
  std::vector<Letter> letters_;
};

#endif  // SRC_SCRABBLE_BAG_H_