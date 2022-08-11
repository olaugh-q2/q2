#include "primes.h"

#include "gmock/gmock-more-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ElementsAreArray;

TEST(PrimesTest, FirstNPrimes) {
  const auto primes = Primes::FirstNPrimes(27);
  EXPECT_THAT(primes,
              ElementsAreArray({0,  2,  3,  5,  7,   11,  13, 17, 19, 23, 29,
                                31, 37, 41, 43, 47,  53,  59, 61, 67, 71, 73,
                                79, 83, 89, 97, 101, 103, 0,  0,  0,  0}));
}