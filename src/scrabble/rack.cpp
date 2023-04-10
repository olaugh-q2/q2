#include "src/scrabble/rack.h"

#include "glog/logging.h"

void Rack::Display(const Tiles& tiles, std::ostream& os) const {
  os << tiles.ToString(letters_).value();
}

namespace {
void AddPowerSets(const Tiles& tiles, const std::array<int, 32>& counts,
                  int index, uint64_t product, LetterString prefix,
                  absl::flat_hash_map<uint64_t, LetterString>* subsets) {
  // LOG(INFO) << "index: " << index << " product: " << product
  //           << " prefix: " << tiles.ToString(prefix).value();
  if (index == tiles.BlankIndex()) {
    return;
  }
  auto new_product = product;
  auto new_prefix = prefix;
  for (int k = 0; k <= counts[index]; ++k) {
    subsets->insert({new_product, new_prefix});
    AddPowerSets(tiles, counts, index + 1, new_product, new_prefix, subsets);
    new_product *= tiles.Prime(index);
    new_prefix.push_back(index);
  }
}

absl::flat_hash_map<uint64_t, LetterString> PowerSet(
    const Tiles& tiles, const std::array<int, 32>& counts) {
  absl::flat_hash_map<uint64_t, LetterString> subsets;
  subsets.reserve(128);  // 2 ** 7
  LetterString prefix;
  AddPowerSets(tiles, counts, 0, 1, prefix, &subsets);
  return subsets;
}
}  // namespace

std::array<int, 32> Rack::Counts() const { return letters_.Counts(); }

absl::flat_hash_map<uint64_t, LetterString> Rack::Subsets(
    const Tiles& tiles) const {
  Products products;
  const auto counts = Counts();
  return PowerSet(tiles, counts);
}

int Rack::NumBlanks(const Tiles& tiles) const {
  int ret = 0;
  for (const auto& letter : letters_) {
    if (letter == tiles.BlankIndex()) {
      ret++;
    }
  }
  return ret;
}

void Rack::RemoveTiles(const LetterString& tiles) {
  auto counts = Counts();
  for (const auto& tile : tiles) {
    const auto i = static_cast<size_t>(tile);
    CHECK_GE(counts[i], 1);
    counts[i]--;
  }
  letters_.clear();
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < counts[i]; ++j) {
      letters_.push_back(i);
    }
  }
}
