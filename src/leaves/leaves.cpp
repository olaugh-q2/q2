#include "src/leaves/leaves.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "glog/logging.h"

std::unique_ptr<Leaves> Leaves::CreateFromCsv(const Tiles& tiles,
                                              const std::string& filename) {
  auto leaves = absl::make_unique<Leaves>();
  std::ifstream input(filename);
  if (!input) {
    LOG(ERROR) << "Failed to open " << filename;
    return nullptr;
  }
  std::string line;
  while (std::getline(input, line)) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ',');
    if (tokens.size() != 2) {
      LOG(ERROR) << "Invalid line (need 2 comma-separated tokens): " << line;
      return nullptr;
    }
    const auto& letters = tokens[0];
    const auto letter_string = tiles.ToLetterString(letters);
    if (!letter_string.has_value()) {
      LOG(INFO) << "Invalid tiles: " << letters;
      return nullptr;
    }
    const absl::uint128 product = tiles.ToProduct(*letter_string);
    absl::uint128 big_threshold = 1;
    big_threshold <<= 64;
    if (product > big_threshold) {
      LOG(INFO) << "Product too large for tiles: " << letters;
      return nullptr;
    }
    const uint64_t small_product = static_cast<uint64_t>(product);

    const auto& float_string = tokens[1];
    float value;
    const bool result = absl::SimpleAtof(float_string, &value);
    if (!result) {
      LOG(INFO) << "Could not parse float: " << float_string;
      return nullptr;
    }
    if (leaves->leave_map_.count(small_product) > 0) {
      LOG(ERROR) << "Duplicate entries for tiles: " << letters;
      return nullptr;
    }
    leaves->leave_map_[small_product] = value;
  }

  return leaves;
}