#include "src/anagram/anagram_map.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "glog/logging.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "src/anagram/anagram_map.pb.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

using google::protobuf::Arena;

std::unique_ptr<AnagramMap> AnagramMap::CreateFromTextfile(
    const Tiles& tiles, const std::string& filename) {
  auto anagram_map = absl::make_unique<AnagramMap>(tiles);
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
  std::vector<std::pair<absl::uint128, std::vector<LetterString>>> sorted_map(
      map.begin(), map.end());
  std::sort(sorted_map.begin(), sorted_map.end(),
            [](const std::pair<absl::uint128, std::vector<LetterString>>& a,
               const std::pair<absl::uint128, std::vector<LetterString>>& b) {
              return a.first < b.first;
            });
  int32_t word_index = 0;
  for (const auto& pair : sorted_map) {
    const auto& product = pair.first;
    const auto& words = pair.second;
    // LOG(INFO) << "product " << product << " has " << words.size() << "
    // words";
    for (const auto& word : words) {
      anagram_map->words_.emplace_back(word);
    }
    anagram_map->word_indices_[product] = word_index;
    word_index += words.size();
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
  std::vector<std::pair<absl::uint128, std::vector<Letter>>> sorted_blank_map(
      blank_map.begin(), blank_map.end());
  std::sort(sorted_blank_map.begin(), sorted_blank_map.end(),
            [](const std::pair<absl::uint128, std::vector<Letter>>& a,
               const std::pair<absl::uint128, std::vector<Letter>>& b) {
              return a.first < b.first;
            });
  uint32_t blank_index = 0;
  for (const auto& pair : sorted_blank_map) {
    const auto& product = pair.first;
    const auto& blanks = pair.second;
    auto sorted_blanks = blanks;
    // LOG(INFO) << "product: " << product;
    std::sort(sorted_blanks.begin(), sorted_blanks.end());
    for (const auto& blank : sorted_blanks) {
      // LOG(INFO) << "  blank: " << tiles.NumberToChar(blank).value();
      anagram_map->blanks_.emplace_back(blank);
    }
    anagram_map->blank_indices_[product] = blank_index;
    blank_index += blanks.size();
    anagram_map->blank_map_[product] =
        absl::MakeConstSpan(anagram_map->blanks_)
            .subspan(anagram_map->blanks_.size() - blanks.size(),
                     blanks.size());
  }
  anagram_map->double_blanks_.reserve(total_double_blanks);
  std::vector<std::pair<absl::uint128, std::vector<LetterPair>>>
      sorted_double_blank_map(double_blank_map.begin(), double_blank_map.end());
  std::sort(sorted_double_blank_map.begin(), sorted_double_blank_map.end(),
            [](const std::pair<absl::uint128, std::vector<LetterPair>>& a,
               const std::pair<absl::uint128, std::vector<LetterPair>>& b) {
              return a.first < b.first;
            });
  uint32_t double_blank_index = 0;
  for (const auto& pair : sorted_double_blank_map) {
    const auto& product = pair.first;
    const auto& blanks = pair.second;
    auto sorted_blanks = blanks;
    std::sort(sorted_blanks.begin(), sorted_blanks.end());
    for (const auto& blank_pair : sorted_blanks) {
      anagram_map->double_blanks_.emplace_back(blank_pair);
    }
    anagram_map->double_blank_indices_[product] = double_blank_index;
    double_blank_index += blanks.size();
    anagram_map->double_blank_map_[product] =
        absl::MakeConstSpan(anagram_map->double_blanks_)
            .subspan(anagram_map->double_blanks_.size() - blanks.size(),
                     blanks.size());
  }
  return anagram_map;
}

std::unique_ptr<AnagramMap> AnagramMap::CreateFromBinaryFile(
    const Tiles& tiles, const std::string& filename) {
  auto anagram_map = absl::make_unique<AnagramMap>(tiles);
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    LOG(ERROR) << "Could not open file " << filename;
    return nullptr;
  }
  auto iis = absl::make_unique<google::protobuf::io::IstreamInputStream>(&file);
  Arena arena;

  auto words = Arena::CreateMessage<q2::proto::Words>(&arena);
  bool clean_eof = true;
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          words, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse words from " << filename;
    return nullptr;
  }
  anagram_map->words_.reserve(words->word_size());
  for (const auto& word : words->word()) {
    const auto letter_string = tiles.ToLetterString(word);
    if (!letter_string) {
      LOG(ERROR) << "Could not parse word " << word;
      return nullptr;
    }
    anagram_map->words_.emplace_back(letter_string.value());
  }

  auto blank_letters = Arena::CreateMessage<q2::proto::BlankLetters>(&arena);
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          blank_letters, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse blank letters from " << filename;
    return nullptr;
  }
  anagram_map->blanks_.reserve(blank_letters->letters().size());
  for (const auto& letter : blank_letters->letters()) {
    anagram_map->blanks_.emplace_back(letter);
  }

  auto double_blank_letters =
      Arena::CreateMessage<q2::proto::DoubleBlankLetters>(&arena);
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          double_blank_letters, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse double blank letters from " << filename;
    return nullptr;
  }
  anagram_map->double_blanks_.reserve(
      double_blank_letters->letter_pairs().size() / 2);
  for (int i = 0; i < double_blank_letters->letter_pairs().size(); i += 2) {
    anagram_map->double_blanks_.emplace_back(
        double_blank_letters->letter_pairs()[i],
        double_blank_letters->letter_pairs()[i + 1]);
  }

  auto word_spans = Arena::CreateMessage<q2::proto::WordSpans>(&arena);
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          word_spans, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse word spans from " << filename;
    return nullptr;
  }
  anagram_map->map_.reserve(word_spans->word_spans_size());
  for (const auto& word_span : word_spans->word_spans()) {
    const absl::uint128 product = absl::MakeUint128(word_span.product().high(),
                                                    word_span.product().low());
    anagram_map->map_[product] =
        absl::MakeConstSpan(anagram_map->words_)
            .subspan(word_span.begin(), word_span.length());
  }

  auto blank_spans = Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          blank_spans, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse blank spans from " << filename;
    return nullptr;
  }
  anagram_map->blank_map_.reserve(blank_spans->blank_spans_size());
  for (const auto& blank_span : blank_spans->blank_spans()) {
    const absl::uint128 product = absl::MakeUint128(blank_span.product().high(),
                                                    blank_span.product().low());
    anagram_map->blank_map_[product] =
        absl::MakeConstSpan(anagram_map->blanks_)
            .subspan(blank_span.begin(), blank_span.length());
  }

  auto double_blank_spans = Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(
          double_blank_spans, iis.get(), &clean_eof)) {
    LOG(ERROR) << "Could not parse double blank spans from " << filename;
    return nullptr;
  }
  anagram_map->double_blank_map_.reserve(
      double_blank_spans->blank_spans_size());
  for (const auto& double_blank_span : double_blank_spans->blank_spans()) {
    const absl::uint128 product = absl::MakeUint128(
        double_blank_span.product().high(), double_blank_span.product().low());
    anagram_map->double_blank_map_[product] =
        absl::MakeConstSpan(anagram_map->double_blanks_)
            .subspan(double_blank_span.begin(), double_blank_span.length());
  }

  return anagram_map;
}

absl::Status AnagramMap::WriteToBinaryFile(const std::string& filename) const {
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    return absl::NotFoundError("Could not open file " + filename);
  }
  if (!WriteToOstream(file)) {
    return absl::InternalError("Could not write to file " + filename);
  }
  return absl::OkStatus();
}

bool AnagramMap::WriteToOstream(std::ostream& os) const {
  Arena arena;
  {
    auto* words = Arena::CreateMessage<q2::proto::Words>(&arena);
    for (const auto& word : words_) {
      words->add_word(tiles_.ToString(word).value());
    }
    // LOG(INFO) << "words: " << words->DebugString();
    if (!google::protobuf::util::SerializeDelimitedToOstream(*words, &os)) {
      return false;
    }
  }
  {
    auto blank_letters = Arena::CreateMessage<q2::proto::BlankLetters>(&arena);
    char* letters = new char[blanks_.size()];
    for (int i = 0; i < blanks_.size(); i++) {
      letters[i] = blanks_[i];
    }
    blank_letters->set_letters(letters, blanks_.size());
    if (!google::protobuf::util::SerializeDelimitedToOstream(*blank_letters,
                                                             &os)) {
      return false;
    }
  }
  {
    auto double_blank_letters =
        Arena::CreateMessage<q2::proto::DoubleBlankLetters>(&arena);
    char* letters = new char[double_blanks_.size() * 2];
    for (int i = 0; i < double_blanks_.size(); i++) {
      letters[i * 2] = double_blanks_[i].first;
      letters[i * 2 + 1] = double_blanks_[i].second;
    }
    double_blank_letters->set_letter_pairs(letters, double_blanks_.size() * 2);
    if (!google::protobuf::util::SerializeDelimitedToOstream(
            *double_blank_letters, &os)) {
      return false;
    }
  }
  {
    auto word_spans = Arena::CreateMessage<q2::proto::WordSpans>(&arena);
    std::vector<std::pair<absl::uint128, absl::Span<const LetterString>>>
        sorted_map(map_.begin(), map_.end());
    std::sort(
        sorted_map.begin(), sorted_map.end(),
        [](const std::pair<absl::uint128, absl::Span<const LetterString>>& a,
           const std::pair<absl::uint128, absl::Span<const LetterString>>& b) {
          return a.first < b.first;
        });
    for (const auto& pair : sorted_map) {
      const auto& product = pair.first;
      const auto& span = pair.second;
      auto* word_span = word_spans->add_word_spans();
      word_span->mutable_product()->set_high(absl::Uint128High64(product));
      word_span->mutable_product()->set_low(absl::Uint128Low64(product));
      word_span->set_begin(word_indices_.at(pair.first));
      word_span->set_length(span.size());
    }
    if (!google::protobuf::util::SerializeDelimitedToOstream(*word_spans,
                                                             &os)) {
      return false;
    }
  }
  {
    auto blank_spans = Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
    std::vector<std::pair<absl::uint128, absl::Span<const Letter>>>
        sorted_blank_map(blank_map_.begin(), blank_map_.end());
    std::sort(sorted_blank_map.begin(), sorted_blank_map.end(),
              [](const std::pair<absl::uint128, absl::Span<const Letter>>& a,
                 const std::pair<absl::uint128, absl::Span<const Letter>>& b) {
                return a.first < b.first;
              });
    for (const auto& pair : sorted_blank_map) {
      const auto& product = pair.first;
      const auto& span = pair.second;
      auto* blank_span = blank_spans->add_blank_spans();
      blank_span->mutable_product()->set_high(absl::Uint128High64(product));
      blank_span->mutable_product()->set_low(absl::Uint128Low64(product));
      blank_span->set_begin(blank_indices_.at(pair.first));
      blank_span->set_length(span.size());
    }
    if (!google::protobuf::util::SerializeDelimitedToOstream(*blank_spans,
                                                             &os)) {
      return false;
    }
  }
  {
    auto double_blank_spans =
        Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
    std::vector<std::pair<absl::uint128, absl::Span<const LetterPair>>>
        sorted_double_blank_map(double_blank_map_.begin(),
                                double_blank_map_.end());
    std::sort(
        sorted_double_blank_map.begin(), sorted_double_blank_map.end(),
        [](const std::pair<absl::uint128, absl::Span<const LetterPair>>& a,
           const std::pair<absl::uint128, absl::Span<const LetterPair>>& b) {
          return a.first < b.first;
        });
    for (const auto& pair : sorted_double_blank_map) {
      const auto& product = pair.first;
      const auto& span = pair.second;
      auto* blank_span = double_blank_spans->add_blank_spans();
      blank_span->mutable_product()->set_high(absl::Uint128High64(product));
      blank_span->mutable_product()->set_low(absl::Uint128Low64(product));
      blank_span->set_begin(double_blank_indices_.at(pair.first));
      blank_span->set_length(span.size());
    }
    if (!google::protobuf::util::SerializeDelimitedToOstream(
            *double_blank_spans, &os)) {
      return false;
    }
  }
  return true;
}

AnagramMap::WordRange AnagramMap::Words(const absl::uint128& product,
                                        int num_blanks) const {
  if (num_blanks == 0) {
    return WordRange(Words(product));
  } else if (num_blanks == 1) {
    std::vector<absl::uint128> products;
    const auto* blanks = Blanks(product);
    if (blanks != nullptr) {
      for (const Letter letter : *blanks) {
        const absl::uint128 new_product = product * tiles_.Prime(letter);
        products.emplace_back(new_product);
      }
    }
    return WordRange(products, *this);
  } else if (num_blanks == 2) {
    std::vector<absl::uint128> products;
    const auto* double_blanks = DoubleBlanks(product);
    if (double_blanks != nullptr) {
      for (const auto& blank_pair : *double_blanks) {
        const absl::uint128 new_product = product *
                                          tiles_.Prime(blank_pair.first) *
                                          tiles_.Prime(blank_pair.second);
        products.emplace_back(new_product);
      }
    }
    return WordRange(products, *this);
  } else {
    LOG(ERROR) << "Invalid number of blanks: " << num_blanks;
    return WordRange(nullptr);
  }
}

std::vector<absl::Span<const LetterString>> AnagramMap::WordRange::MakeSpans(
    const absl::Span<const LetterString>* span) const {
  if (span == nullptr) {
    return {};
  } else {
    return {*span};
  }
}

std::vector<absl::Span<const LetterString>> AnagramMap::WordRange::MakeSpans(
    const std::vector<absl::uint128>& products,
    const AnagramMap& anagram_map) const {
  std::vector<absl::Span<const LetterString>> spans;
  spans.reserve(products.size());
  for (const auto& product : products) {
    const auto* span = anagram_map.Words(product);
    if (span != nullptr) {
      spans.emplace_back(*span);
    } else {
      LOG(ERROR) << "Invalid product with no words: " << product;
    }
  }
  return spans;
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
