#include "src/scrabble/tiles.h"

#include <iterator>

#include "glog/logging.h"
#include "src/scrabble/primes.h"
#include "src/scrabble/strings.h"

Tiles::Tiles(const std::string& distribution)
    : blank_index_(FindBlankIndex(distribution)),
      distribution_(DistributionFromString(distribution)),
      primes_(TilePrimes(Primes::FirstNPrimes(blank_index_), PrimeIndices())) {}

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

int Tiles::Count(int number) const { return distribution_[number]; }

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

absl::optional<LetterString> Tiles::ToLetterString(const std::string& s) const {
  LetterString ret;
  for (size_t i = 0; i < s.size(); i++) {
    const char c = s[i];
    auto number = CharToNumber(c);
    if (number.has_value()) {
      ret.push_back(number.value());
    } else {
      return absl::nullopt;
    }
  }
  return ret;
}

absl::optional<std::string> Tiles::ToString(const LetterString& s) const {
  std::string ret;
  for (char n : s) {
    auto c = NumberToChar(n);
    if (c.has_value()) {
      ret.push_back(c.value());
    } else {
      return absl::nullopt;
    }
  }
  return ret;
}

std::array<uint64_t, 32> Tiles::TilePrimes(const std::array<uint64_t, 32>& primes,
                                      const std::array<int, 32>& indices) {
  std::array<uint64_t, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (size_t i = 0; i < indices.size(); i++) {
    const int index = indices[i];
    if (index != 0) {
      ret[i] = primes[index];
    }
  }
  return ret;
}

std::array<int, 32> Tiles::PrimeIndices() {
  std::vector<std::pair<int, int>> counts;
  for (size_t i = 0; i < distribution_.size(); i++) {
    if (distribution_[i] > 0) {
      counts.push_back(std::make_pair(distribution_[i], i));
    }
  }
  std::sort(std::begin(counts), std::end(counts),
            [this](const std::pair<int, int>& a, const std::pair<int, int>& b) {
              // Blank is last regardless of count. It gets the biggest prime
              // because the maps containing large multisets don't have blanks
              // in their keys. I believe only maps for leave values and other
              // things with 7 or fewer tiles will need to contain blanks.
              if (a.second == this->blank_index_) {
                return false;
              }
              if (b.second == this->blank_index_) {
                return true;
              }
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

absl::uint128 Tiles::ToProduct(const LetterString& s) const {
  absl::uint128 ret = 1;
  for (char c : s) {
    ret *= primes_[c];
  }
  return ret;
}