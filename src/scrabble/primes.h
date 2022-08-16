#ifndef SRC_SCRABBLE_PRIMES_H_
#define SRC_SCRABBLE_PRIMES_H_

#include <array>

class Primes {
 public:
  static std::array<uint64_t, 32> FirstNPrimes(int n);
};

#endif  // SRC_SCRABBLE_PRIMES_H_