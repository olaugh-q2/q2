#include "src/leaves/leaves.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "glog/logging.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "src/leaves/leaves.pb.h"
#include "src/scrabble/tiles.h"

using google::protobuf::Arena;

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

std::unique_ptr<Leaves> Leaves::CreateFromBinaryFile(
    const Tiles& tiles, const std::string& filename) {
  auto leaves = absl::make_unique<Leaves>();
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    LOG(ERROR) << "Could not open file " << filename;
    return nullptr;
  }
  auto iis = absl::make_unique<google::protobuf::io::IstreamInputStream>(&file);
  Arena arena;

  auto proto_leaves = Arena::CreateMessage<q2::proto::Leaves>(&arena);
  bool clean_eof = true;
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          proto_leaves, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse file " << filename;
    return nullptr;
  }
  leaves->leave_map_.reserve(proto_leaves->leaves_size());
  for (const auto& leave : proto_leaves->leaves()) {
    const auto& product = leave.product();
    const auto& value = leave.value();
    if (leaves->leave_map_.count(product) > 0) {
      LOG(ERROR) << "Duplicate entries for product: " << product;
      return nullptr;
    }
    leaves->leave_map_[product] = value;
  }
  return leaves;
}

absl::Status Leaves::WriteToBinaryFile(const std::string& filename) const {
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    return absl::NotFoundError("Could not open file " + filename);
  }
  if (!WriteToOstream(file)) {
    return absl::InternalError("Could not write to file " + filename);
  }
  return absl::OkStatus();
}

bool Leaves::WriteToOstream(std::ostream& os) const {
  Arena arena;
  auto leaves = Arena::CreateMessage<q2::proto::Leaves>(&arena);
  for (const auto& entry : leave_map_) {
    auto* leave = leaves->add_leaves();
    leave->set_product(entry.first);
    leave->set_value(entry.second);
  }
  if (!google::protobuf::util::SerializeDelimitedToOstream(*leaves, &os)) {
    return false;
  }
  return true;
}