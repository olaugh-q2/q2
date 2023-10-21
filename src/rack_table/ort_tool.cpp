#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/str_join.h"
#include "glog/logging.h"
#include "src/rack_table/magpie_rack.h"

ABSL_FLAG(std::string, input_wordlist, "", "input .txt file path");
ABSL_FLAG(std::string, output_word_sizes, "", "output .txt file path");
ABSL_FLAG(int, num_buckets, 0, "number of hash table buckets");

ABSL_FLAG(std::string, input_word_sizes, "", "input .txt file path");
ABSL_FLAG(std::string, output_ort, "", "output .ort file path");

bool IsPrime(int num_buckets) {
  if (num_buckets < 2) {
    return false;
  }
  for (int i = 2; i * i <= num_buckets; ++i) {
    if (num_buckets % i == 0) {
      return false;
    }
  }
  return true;
}

int GetDivisor(int num_buckets) {
  for (int i = 2; i * i <= num_buckets; ++i) {
    if (num_buckets % i == 0) {
      return i;
    }
  }
  return 1;
}

void WriteOrt(const std::string& input, const std::string& output,
              uint64_t num_buckets) {
  std::ifstream input_stream(input);
  std::ofstream output_stream(output);

  absl::flat_hash_map<uint32_t,
                      std::vector<std::pair<uint16_t, std::array<int, 6>>>>
      map;
  CHECK(num_buckets < (1UL << 32));
  const uint32_t num_buckets_uint32 = static_cast<uint32_t>(num_buckets);
  output_stream.write(reinterpret_cast<const char*>(&num_buckets_uint32),
                      sizeof(num_buckets_uint32));

  uint64_t num_lines = 0;
  std::string line;
  while (std::getline(input_stream, line)) {
    std::istringstream iss(line);
    uint64_t rack_uint64;
    iss >> rack_uint64;
    std::array<int, 6> word_sizes;
    for (int i = 0; i < 6; ++i) {
      iss >> word_sizes[i];
    }
    int quotient = rack_uint64 / num_buckets;
    int bucket = rack_uint64 % num_buckets;
    CHECK(quotient < (1 << 14));
    CHECK(bucket < (1UL << 32));
    map[bucket].push_back(std::make_pair(quotient, word_sizes));
    num_lines++;
  }
  LOG(INFO) << "num_lines: " << num_lines;
  CHECK(num_lines < (1UL << 32));
  const uint32_t num_values_uint32 = static_cast<uint32_t>(num_lines);
  output_stream.write(reinterpret_cast<const char*>(&num_values_uint32),
                      sizeof(num_values_uint32));
  uint32_t bucket_start = 0;
  for (int i = 0; i < num_buckets; ++i) {
    const auto& bucket = map[i];
    const uint32_t bucket_size = static_cast<uint32_t>(bucket.size());
    output_stream.write(reinterpret_cast<const char*>(&bucket_start),
                        sizeof(bucket_start));
    bucket_start += bucket_size;
  }
  output_stream.write(reinterpret_cast<const char*>(&bucket_start),
                      sizeof(bucket_start));
  for (int i = 0; i < num_buckets; ++i) {
    auto values = map[i];
    // sort by quotient
    std::sort(values.begin(), values.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& value : values) {
      uint32_t packed_value = 0;
      for (int j = 0; j < 6; j++) {
        int word_size = value.second[j];
        CHECK(word_size < 8);
        packed_value |= word_size << (14 + (3 * j));
      }
      packed_value |= value.first;
      output_stream.write(reinterpret_cast<const char*>(&packed_value),
                          sizeof(packed_value));
    }
  }
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  const std::string input_wordlist = absl::GetFlag(FLAGS_input_wordlist);
  const std::string input_word_sizes = absl::GetFlag(FLAGS_input_word_sizes);
  const int num_buckets = absl::GetFlag(FLAGS_num_buckets);
  if (!input_word_sizes.empty()) {
    const bool is_prime = IsPrime(num_buckets);
    if (!is_prime) {
      if (num_buckets > 3) {
        const int divisor = GetDivisor(num_buckets);
        std::cout << "num_buckets must be prime, num_buckets = " << divisor
                  << " * " << num_buckets / divisor << std::endl;
      }
      std::cout << "num_buckets must be prime" << std::endl;
      return 1;
    }
    WriteOrt(input_word_sizes, absl::GetFlag(FLAGS_output_ort), num_buckets);
    return 0;
  }

  const MagpieRack english_bag = MagpieRackFromString(
      "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
      "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");

  std::ifstream input(input_wordlist);
  std::string word;
  std::vector<std::string> words;
  while (std::getline(input, word)) {
    words.push_back(word);
  }

  const int rack_size = 7;
  const int max_playthrough = 5;
  const auto map =
      MakeRackWordSizesMap(words, english_bag, rack_size, max_playthrough);
  const std::string output_word_sizes = absl::GetFlag(FLAGS_output_word_sizes);
  std::ofstream output(output_word_sizes);
  for (const auto& pair : map) {
    const auto& key = pair.first;
    const auto& value = pair.second;
    const auto kth = std::next(value.begin(), max_playthrough + 1);
    const std::string value_string = absl::StrJoin(value.begin(), kth, " ");
    output << key << " " << value_string << std::endl;
  }
  return 0;
}