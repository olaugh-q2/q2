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
  anagram_map->words_.reserve(num_words);
  for (const auto& pair : map) {
    const auto& product = pair.first;
    const auto& words = pair.second;
    for (const auto& word : words) {
      anagram_map->words_.emplace_back(word);
    }
    anagram_map->map_[product] =
        absl::MakeConstSpan(anagram_map->words_)
            .subspan(anagram_map->words_.size() - words.size(), words.size());
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