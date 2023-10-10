#include "src/scrabble/primes.h"

#include <iterator>

#include "glog/logging.h"

std::array<uint64_t, 32> Primes::FirstNPrimes(int n) {
  std::array<uint64_t, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  ret[1] = 2;
  int i = 1;
  for (int x = 3;; x += 2) {
    bool is_prime = true;
    for (int d_i = 1; static_cast<int>(ret[d_i] * ret[d_i]) <= x; d_i++) {
      if (x % ret[d_i] == 0) {
        is_prime = false;
        break;
      }
    }
    if (is_prime) {
      ret[++i] = x;
      if (i == n) {
        return ret;
      }
    }
  }
  return ret;
}