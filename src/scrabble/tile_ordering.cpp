#include "src/scrabble/tile_ordering.h"

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