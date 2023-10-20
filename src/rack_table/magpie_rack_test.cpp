#include "src/rack_table/magpie_rack.h"

#include <gtest/gtest.h>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_join.h"
#include "glog/logging.h"
#include "gmock/gmock.h"

using ::testing::ElementsAre;

TEST(MagpieRackTest, GetSubselections) {
  const MagpieRack english_bag = MagpieRackFromString(
      "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
      "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");
  EXPECT_EQ(english_bag.num_elements, 100);
}

TEST(MagpieRackTest, ToUint64) {
  const MagpieRack aab_ = MagpieRackFromString("AAB?");
  const uint64_t aab__uint64 = MagpieRackToUint64(aab_);
  EXPECT_EQ(aab__uint64, 33883);
}

namespace {
void BucketWithModulus(const std::vector<MagpieRack>& subselections,
                       int* used_buckets, int* max_bucket_size,
                       uint64_t modulus) {
  absl::flat_hash_map<uint64_t, int> map;
  *used_buckets = 0;
  *max_bucket_size = 0;
  for (const auto& subselection : subselections) {
    const uint64_t key = MagpieRackToUint64(subselection) % modulus;
    map[key]++;
  }
  for (const auto& pair : map) {
    const int value = pair.second;
    if (value > 0) {
      (*used_buckets)++;
    }
    if (value > *max_bucket_size) {
      *max_bucket_size = value;
    }
  }
}

std::vector<uint64_t> PrimesBetween(uint64_t min, uint64_t max) {
  std::vector<bool> is_prime(max + 1, true);
  for (int i = 2; i * i <= max; ++i) {
    if (is_prime[i]) {
      for (int j = i * i; j <= max; j += i) {
        is_prime[j] = false;
      }
    }
  }
  std::vector<uint64_t> primes;
  for (int i = min; i <= max; ++i) {
    if (is_prime[i] && i >= min) {
      primes.push_back(i);
    }
  }
  return primes;
}
}  // namespace
TEST(MagpieRackTest, HashTest) {
  const MagpieRack english_bag = MagpieRackFromString(
      "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
      "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");
  const auto subselections = GetSubselections(english_bag, 7);
  EXPECT_EQ(subselections.size(), 4114349);
  int used_buckets;
  int max_bucket_size;

  BucketWithModulus(subselections, &used_buckets, &max_bucket_size, 5300003);
  EXPECT_EQ(used_buckets, 2972483);
  EXPECT_EQ(max_bucket_size, 7);

  const auto primes =
      PrimesBetween(5000000, 5000000);  // Empty list to disable output
  int most_buckets = 0;
  for (const auto& prime : primes) {
    BucketWithModulus(subselections, &used_buckets, &max_bucket_size, prime);
    if (used_buckets > most_buckets) {
      most_buckets = used_buckets;
      LOG(INFO) << "prime: " << prime << " used_buckets: " << used_buckets
                << " max_bucket_size: " << max_bucket_size << std::endl;
    }
  }

  BucketWithModulus(subselections, &used_buckets, &max_bucket_size, 5297687);
  EXPECT_EQ(used_buckets, 3213586);
  EXPECT_EQ(max_bucket_size, 5);
}

namespace {
uint64_t KeyFromString(const std::string& letters) {
  MagpieRack rack = MagpieRackFromString(letters);
  return MagpieRackToUint64(rack);
}
}  // namespace

TEST(MagpieRackTest, MakeRackWordSizesMap) {
  const MagpieRack english_bag = MagpieRackFromString(
      "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
      "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");
  const auto map =
      MakeRackWordSizesMap({"AA", "AB", "BACK"}, english_bag, 4, 3);
  /*
  for (const auto& pair : map) {
    const auto& key = pair.first;
    const auto& value = pair.second;
    const std::string value_string = absl::StrJoin(value, ",");
    LOG(INFO) << "key: " << key << " value: " << value_string << std::endl;
  }
  */

  // Shouldn't put an empty subrack in the map. In Scrabble you can't make words
  // by adding _nothing_ to an existing word, so having this in the map could
  // only lead to misuse.
  const auto empty_key = KeyFromString("");
  auto it = map.find(empty_key);
  EXPECT_TRUE(it == map.end());

  const auto a_key = KeyFromString("A");
  it = map.find(a_key);
  EXPECT_THAT(it->second, testing::ElementsAre(0, 1, 0, 1, 0, 0, 0, 0));

  const auto aa_key = KeyFromString("AA");
  it = map.find(aa_key);
  EXPECT_THAT(it->second, testing::ElementsAre(2, 1, 0, 1, 0, 0, 0, 0));

  const auto k_key = KeyFromString("K");
  it = map.find(k_key);
  EXPECT_THAT(it->second, testing::ElementsAre(0, 0, 0, 1, 0, 0, 0, 0));

  const auto back_key = KeyFromString("BACK");
  it = map.find(back_key);
  EXPECT_THAT(it->second, testing::ElementsAre(4, 3, 2, 1, 0, 0, 0, 0));

  const auto blank_key = KeyFromString("?");
  it = map.find(blank_key);
  EXPECT_THAT(it->second, testing::ElementsAre(0, 1, 0, 1, 0, 0, 0, 0));

  const auto b_blank_key = KeyFromString("B?");
  it = map.find(b_blank_key);
  EXPECT_THAT(it->second, testing::ElementsAre(2, 1, 2, 1, 0, 0, 0, 0));

  const auto c_2blank_key = KeyFromString("C??");
  it = map.find(c_2blank_key);
  EXPECT_THAT(it->second, testing::ElementsAre(2, 3, 2, 1, 0, 0, 0, 0));
}