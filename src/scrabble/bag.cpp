#include "src/scrabble/bag.h"

#include "glog/logging.h"

Bag::Bag(const Tiles& tiles) : tiles_(tiles) {
  for (Letter letter = tiles_.FirstLetter(); letter <= tiles_.BlankIndex();
       ++letter) {
    for (int i = 0; i < tiles_.Count(letter); ++i) {
      letters_.push_back(letter);
    }
  }
}

void Bag::Display(std::ostream& os) const {
  for (Letter letter : letters_) {
    os << tiles_.NumberToChar(letter).value();
  }
}