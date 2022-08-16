#ifndef SRC_ANAGRAM_ANAGRAM_MAP_H_
#define SRC_ANAGRAM_ANAGRAM_MAP_H_

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"

class AnagramMap {
 public:
  static std::unique_ptr<AnagramMap> CreateFromTextfile(const std::string& filename);

 private:
  absl::flat_hash_map<uint64_t, std::vector<std::string>> natural_small_;
  absl::flat_hash_map<absl::uint128, std::vector<std::string>> natural_large_;
};

#endif  // SRC_ANAGRAM_ANAGRAM_MAP_H_