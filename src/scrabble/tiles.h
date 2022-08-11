#include <array>
#include <string>

#include "absl/types/optional.h"
#include "gtest/gtest_prod.h"
class Tiles {
 public:
  Tiles(const std::string& distribution);
  absl::optional<int> CharToNumber(char c) const;
  int Count(int number) const;

 private:
  FRIEND_TEST(TilesTest, PrimeIndices);

  int FindBlankIndex(const std::string& distribution);
  std::array<int, 32> DistributionFromString(const std::string& distribution);
  std::array<int, 32> PrimeIndices();

  const int blank_index_;
  const std::array<int, 32> distribution_;
  const std::array<uint64_t, 32> primes_;
};