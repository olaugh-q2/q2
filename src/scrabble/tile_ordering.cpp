#include "src/scrabble/tile_ordering.h"

#include <absl/random/random.h> 

#include "src/scrabble/bag.h"
#include "src/scrabble/data_manager.h"

namespace {
std::vector<Letter> GetLettersFromProto(const q2::proto::TileOrdering& proto,
                                        const Tiles& tiles) {
  std::vector<Letter> letters;
  letters.reserve(proto.letters().size());
  for (const auto& c : proto.letters()) {
    const auto letter = tiles.CharToNumber(c);
    if (!letter.has_value()) {
      LOG(ERROR) << "Invalid letter in TileOrdering proto: " << c;
      return {};
    }
    letters.push_back(*letter);
  }
  return letters;
}

std::vector<uint16_t> GetDividendsFromProto(
    const q2::proto::TileOrdering& proto) {
  std::vector<uint16_t> dividends;
  dividends.reserve(proto.exchange_insertion_dividends_size());
  for (const auto& dividend : proto.exchange_insertion_dividends()) {
    dividends.push_back(dividend);
  }
  return dividends;
}
}  // namespace

TileOrdering::TileOrdering(const q2::proto::TileOrdering& proto,
                           const Tiles& tiles)
    : letters_(GetLettersFromProto(proto, tiles)),
      exchange_insertion_dividends_(GetDividendsFromProto(proto)) {}

namespace {
std::vector<Letter> Shuffle(const Tiles& tiles, absl::BitGenRef gen) {
  Bag bag(tiles);
  bag.Shuffle(gen);
  return bag.Letters();
}

std::vector<uint16_t> RandomDividends(absl::BitGenRef gen, int size) {
  std::vector<uint16_t> dividends;
  dividends.reserve(size);
  for (int i = 0; i < size; ++i) {
    dividends.push_back(absl::Uniform<uint16_t>(gen, 0, 65535));
  }
  return dividends;
}
}  // namespace
TileOrdering::TileOrdering(const Tiles& tiles, absl::BitGenRef gen,
                           int num_dividends)
    : letters_(Shuffle(tiles, gen)),
      exchange_insertion_dividends_(RandomDividends(gen, num_dividends)) {}

TileOrdering TileOrdering::Adjust(
    const std::vector<Letter>& used_letters) const {
  std::vector<Letter> letters = letters_;
  // Remove used_letters from letters ordering, taking away letters at the back
  // of the list first.
  for (Letter letter : used_letters) {
    auto it = std::find(letters.rbegin(), letters.rend(), letter);
    if (it == letters.rend()) {
      LOG(ERROR) << "Letter " << letter << " not found in letters";
      return TileOrdering({}, {});
    }
    letters.erase((it + 1).base());
  }
  return TileOrdering(letters, exchange_insertion_dividends_);
}