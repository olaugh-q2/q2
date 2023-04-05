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

std::unique_ptr<Bag> Bag::UnseenToPlayer(const Tiles& tiles, const Board& board,
                                         const Rack& rack) {
  std::array<int, 32> counts = rack.Counts();
  std::fill(counts.begin(), counts.end(), 0);
  for (Letter letter = tiles.FirstLetter(); letter <= tiles.BlankIndex();
       ++letter) {
    counts[letter] = tiles.Count(letter);
  }
  for (int row = 0; row < 15; ++row) {
    for (int col = 0; col < 15; ++col) {
      if (const Letter letter = board.At(row, col)) {
        counts[letter]--;
        CHECK_GE(counts[letter], 0);
      }
    }
  }
  for (const Letter& letter : rack.Letters()) {
    counts[letter]--;
    CHECK_GE(counts[letter], 0);
  }
  std::vector<Letter> letters;
  for (Letter letter = tiles.FirstLetter(); letter <= tiles.BlankIndex();
       ++letter) {
    for (int i = 0; i < counts[letter]; ++i) {
      letters.push_back(letter);
    }
  }
  return std::unique_ptr<Bag>(new Bag(tiles, letters));
}

void Bag::Display(std::ostream& os) const {
  for (Letter letter : letters_) {
    os << tiles_.NumberToChar(letter).value();
  }
}