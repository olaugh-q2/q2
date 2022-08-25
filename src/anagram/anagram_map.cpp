#include "src/anagram/anagram_map.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "glog/logging.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

std::unique_ptr<AnagramMap> AnagramMap::CreateFromTextfile(
    const Tiles& tiles, const std::string& filename) {
  auto anagram_map = absl::make_unique<AnagramMap>();
  std::ifstream input(filename);
  if (!input) {
    LOG(ERROR) << "Failed to open " << filename;
    return nullptr;
  }
  std::string word;
  absl::flat_hash_map<absl::uint128, std::vector<LetterString>> map;
  int num_words = 0;
  while (std::getline(input, word)) {
    const auto letter_string = tiles.ToLetterString(word);
    if (!letter_string) {
      LOG(ERROR) << "Failed to read word " << word;
      return nullptr;
    }
    const absl::uint128 product = tiles.ToProduct(letter_string.value());
    map[product].push_back(letter_string.value());
    num_words++;
  }
  absl::flat_hash_map<absl::uint128, absl::Span<const LetterString>> map_;
  anagram_map->words_.reserve(num_words);
  absl::flat_hash_map<absl::uint128, std::vector<Letter>> blank_map;
  absl::flat_hash_map<absl::uint128, std::vector<LetterPair>> double_blank_map;
  int total_blanks = 0;
  int total_double_blanks = 0;
  for (const auto& pair : map) {
    const auto& product = pair.first;
    const auto& words = pair.second;
    for (const auto& word : words) {
      anagram_map->words_.emplace_back(word);
    }
    anagram_map->map_[product] =
        absl::MakeConstSpan(anagram_map->words_)
            .subspan(anagram_map->words_.size() - words.size(), words.size());
    for (Letter letter = 1; letter <= 26; letter++) {
      const auto prime = tiles.Prime(letter);
      // LOG(INFO) << "prime: " << prime;
      if (product % prime == 0) {
        const auto subproduct = product / prime;
        blank_map[subproduct].push_back(letter);
        total_blanks++;
        // LOG(INFO) << "blank: " << tiles.NumberToChar(letter).value();
        for (Letter letter2 = letter; letter2 <= 26; letter2++) {
          const auto prime2 = tiles.Prime(letter2);
          if (subproduct % prime2 == 0) {
            double_blank_map[subproduct / prime2].push_back(
                std::make_pair(letter, letter2));
            total_double_blanks++;
          }
        }
      }
    }
  }
  anagram_map->blanks_.reserve(total_blanks);
  for (const auto& pair : blank_map) {
    const auto& product = pair.first;
    const auto& blanks = pair.second;
    auto sorted_blanks = blanks;
    std::sort(sorted_blanks.begin(), sorted_blanks.end());
    for (const auto& blank : sorted_blanks) {
      anagram_map->blanks_.emplace_back(blank);
    }
    anagram_map->blank_map_[product] =
        absl::MakeConstSpan(anagram_map->blanks_)
            .subspan(anagram_map->blanks_.size() - blanks.size(),
                     blanks.size());
  }
  anagram_map->double_blanks_.reserve(total_double_blanks);
  for (const auto& pair : double_blank_map) {
    const auto& product = pair.first;
    const auto& blanks = pair.second;
    auto sorted_blanks = blanks;
    std::sort(sorted_blanks.begin(), sorted_blanks.end());
    for (const auto& blank_pair : sorted_blanks) {
      anagram_map->double_blanks_.emplace_back(blank_pair);
    }
    anagram_map->double_blank_map_[product] =
        absl::MakeConstSpan(anagram_map->double_blanks_)
            .subspan(anagram_map->double_blanks_.size() - blanks.size(),
                     blanks.size());
  }
  return anagram_map;
}

const absl::Span<const LetterString>* AnagramMap::Words(
    const absl::uint128& product) const {
  const auto it = map_.find(product);
  if (it == map_.end()) {
    return nullptr;
  }
  return &it->second;
}

const absl::Span<const Letter>* AnagramMap::Blanks(
    const absl::uint128& product) const {
  const auto it = blank_map_.find(product);
  if (it == blank_map_.end()) {
    return nullptr;
  }
  return &it->second;
}

const absl::Span<const LetterPair>* AnagramMap::DoubleBlanks(
    const absl::uint128& product) const {
  const auto it = double_blank_map_.find(product);
  if (it == double_blank_map_.end()) {
    return nullptr;
  }
  return &it->second;
}
