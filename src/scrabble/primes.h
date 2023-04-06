#ifndef SRC_SCRABBLE_PRIMES_H
#define SRC_SCRABBLE_PRIMES_H

#include <array>

class Primes {
 public:
  static std::array<uint64_t, 32> FirstNPrimes(int n);
};

#endif  // SRC_SCRABBLE_PRIMES_H