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

Bag Bag::UnseenToPlayer(const Board& board, const Rack& rack) const {
  //LOG(INFO) << "UnseenToPlayer()";
  //std::stringstream ss2;
  //rack.Display(tiles_, ss2);
  //LOG(INFO) << "rack: " << std::endl << ss2.str();
  //std::stringstream ss3;
  //Display(ss3);
  //LOG(INFO) << "(this) bag: " << std::endl << ss3.str();
  
  std::array<int, 32> counts = rack.Counts();
  std::fill(std::begin(counts), std::end(counts), 0);
  for (Letter letter : letters_) {
    counts[letter]++;
  }
  for (int i = 0; i < 32; i++) {
    //LOG(INFO) << "counts[" << i << "] = " << counts[i];
  }
  for (int row = 0; row < 15; ++row) {
    for (int col = 0; col < 15; ++col) {
      if (Letter letter = board.At(row, col)) {
        if (letter >= tiles_.BlankIndex()) {
          letter = tiles_.BlankIndex();
        }
        //LOG(INFO) << "decrementing board letter: " << tiles_.NumberToChar(letter).value();
        counts[letter]--;
        CHECK_GE(counts[letter], 0) << "negative count of letter: " << tiles_.NumberToChar(letter).value();
      }
    }
  }
  for (const Letter& letter : rack.Letters()) {
    counts[letter]--;
    CHECK_GE(counts[letter], 0) << "couldn't safely decrement count of letter: " << tiles_.NumberToChar(letter).value();
  }
  std::vector<Letter> letters;
  for (Letter letter = tiles_.FirstLetter(); letter <= tiles_.BlankIndex();
       ++letter) {
    for (int i = 0; i < counts[letter]; ++i) {
      letters.push_back(letter);
    }
  }
  //for (int i = 0; i < 32; i++) {
  //  LOG(INFO) << "counts[" << i << "] = " << counts[i];
  //}
  const Bag bag(tiles_, letters);
  return bag;
}

void Bag::Display(std::ostream& os) const {
  for (Letter letter : letters_) {
    os << tiles_.NumberToChar(letter).value();
  }
}

std::string Bag::ToString() const {
  std::ostringstream os;
  Display(os);
  return os.str();
}

void Bag::CompleteRack(Rack* rack) {
  while ((rack->NumTiles() < rack->Capacity()) && !letters_.empty()) {
    rack->PushBack(letters_.back());
    letters_.pop_back();
  }
}

void Bag::InsertTiles(const LetterString& tiles,
                      const std::vector<uint16_t>& exchange_insertion_dividends,
                      std::size_t* exchange_insertion_index) {
  if (tiles.empty()) {
    return;
  }                        
  for (const Letter& letter : tiles) {
    const int index = exchange_insertion_dividends[*exchange_insertion_index] %
                      (letters_.size() + 1);
    letters_.insert(letters_.begin() + index, letter);
    (*exchange_insertion_index)++;
    if (*exchange_insertion_index >= exchange_insertion_dividends.size()) {
      *exchange_insertion_index = 0;
    }
  }
}