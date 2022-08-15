#include "tiles.h"

#include <iterator>

#include "glog/logging.h"
#include "primes.h"

Tiles::Tiles(const std::string& distribution)
    : blank_index_(FindBlankIndex(distribution)),
      distribution_(DistributionFromString(distribution)),
      primes_(Primes::FirstNPrimes(blank_index_)) {}

std::array<int, 32> Tiles::DistributionFromString(
    const std::string& distribution) {
  std::array<int, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (size_t i = 0; i < distribution.size(); i++) {
    const char c = distribution[i];
    auto number = CharToNumber(c);
    if (number.has_value()) {
      ret[number.value()]++;
    }
  }
  return ret;
}

int Tiles::FindBlankIndex(const std::string& distribution) {
  return 'Z' - 'A' + 2;  // 27
}

absl::optional<int> Tiles::CharToNumber(char c) const {
  if (c == '?') {
    return blank_index_;
  }
  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 1;
  }
  LOG(ERROR) << "Could not convert character '" << c << "' to number";
  return absl::nullopt;
}

int Tiles::Count(int number) const {
  return distribution_[number];
}

absl::optional<char> Tiles::NumberToChar(int n) const {
  if (n == blank_index_) {
    return '?';
  }
  if (n >= 1 && n <= 26) {
    return n + 'A' - 1;
  }
  LOG(ERROR) << "Could not convert number " << n << " to character";
  return absl::nullopt;
}

std::array<int, 32> Tiles::PrimeIndices() {
  std::vector<std::pair<int, int>> counts;
  for (size_t i = 0; i < distribution_.size(); i++) {
    if (distribution_[i] > 0) {
      counts.push_back(std::make_pair(distribution_[i], i));
    }
  }
  std::sort(std::begin(counts), std::end(counts),
            [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
              if (a.first == b.first) {
                return a.second < b.second;
              }
              return a.first > b.first;
            });

  std::array<int, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (size_t i = 0; i < counts.size(); i++) {
    ret[counts[i].second] = i + 1;
  }
  return ret;
}