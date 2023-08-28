#ifndef SRC_SCRABBLE_TILES_H
#define SRC_SCRABBLE_TILES_H

#include <array>
#include <codecvt>
#include <locale>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "absl/types/optional.h"
#include "gtest/gtest_prod.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles_spec.pb.h"

class Tiles {
 public:
  Tiles(const std::string& filename);
  absl::optional<Letter> CharToNumber(char c) const;
  absl::optional<char> NumberToChar(Letter letter) const;
  absl::optional<LetterString> ToLetterString(const std::string& s) const;
  absl::optional<std::string> ToString(const LetterString& s) const;
  LetterString Unblank(const LetterString& s) const;
  uint64_t Prime(Letter letter) const;
  absl::uint128 ToProduct(const LetterString& s) const;
  uint64_t ToProduct64(const LetterString& s) const;
  int Count(Letter letter) const;
  char32_t FullWidth(Letter letter) const {
    return fullwidth_symbols_[static_cast<int>(letter)];
  }
  inline int FirstLetter() const { return 1; }  // A
  int BlankIndex() const { return blank_index_; }
  inline int Score(Letter letter) const { return scores_[static_cast<int>(letter)]; }
  int Score(const LetterString& s) const {
    int ret = 0;
    for (Letter letter : s) {
      ret += Score(letter);
    }
    return ret;
  }

 private:
  FRIEND_TEST(TilesTest, PrimeIndices);

  absl::optional<q2::proto::TilesSpec> LoadTilesSpec(
      const std::string& filename);
  std::array<char32_t, 64> MakeFullwidthSymbols() const;
  std::array<int, 32> DistributionFromProto(const q2::proto::TilesSpec& proto);
  std::array<int, 32> ScoresFromProto(const q2::proto::TilesSpec& proto);
  int FindBlankIndex(const q2::proto::TilesSpec& proto);
  std::array<int, 32> PrimeIndices();

  std::array<uint64_t, 32> TilePrimes(const std::array<uint64_t, 32>& primes,
                                      const std::array<int, 32>& indices);

  q2::proto::TilesSpec proto_;
  const int blank_index_;
  const std::array<char32_t, 64> fullwidth_symbols_;
  const std::array<int, 32> distribution_;
  const std::array<int, 32> scores_;
  const std::array<uint64_t, 32> primes_;
};

inline uint64_t Tiles::Prime(Letter letter) const { return primes_[letter]; }

#endif  // SRC_SCRABBLE_TILES_H