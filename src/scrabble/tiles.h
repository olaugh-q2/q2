#ifndef SRC_SCRABBLE_TILES_H_
#define SRC_SCRABBLE_TILES_H_

#include <array>
#include <string>

#include "absl/numeric/int128.h"
#include "absl/types/optional.h"
#include "gtest/gtest_prod.h"
#include "src/scrabble/strings.h"

class Tiles {
 public:
  Tiles(const std::string& distribution);
  absl::optional<Letter> CharToNumber(char c) const;
  absl::optional<char> NumberToChar(Letter letter) const;
  absl::optional<LetterString> ToLetterString(const std::string& s) const;
  absl::optional<std::string> ToString(const LetterString& s) const;
  uint64_t Prime(Letter letter) const;
  absl::uint128 ToProduct(const LetterString& s) const;
  int Count(Letter letter) const;

 private:
  FRIEND_TEST(TilesTest, PrimeIndices);

  int FindBlankIndex(const std::string& distribution);
  std::array<int, 32> DistributionFromString(const std::string& distribution);
  std::array<int, 32> PrimeIndices();
  std::array<uint64_t, 32> TilePrimes(const std::array<uint64_t, 32>& primes,
                                      const std::array<int, 32>& indices);

  const int blank_index_;
  const std::array<int, 32> distribution_;
  const std::array<uint64_t, 32> primes_;
};

inline uint64_t Tiles::Prime(Letter letter) const { return primes_[letter]; }

#endif  // SRC_SCRABBLE_TILES_H_