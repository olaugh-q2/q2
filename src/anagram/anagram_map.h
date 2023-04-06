#ifndef SRC_ANAGRAM_ANAGRAM_MAP_H
#define SRC_ANAGRAM_ANAGRAM_MAP_H

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

typedef std::pair<Letter, Letter> LetterPair;
typedef absl::flat_hash_map<
    absl::uint128, absl::Span<const LetterString>>::const_iterator WordIterator;
typedef absl::flat_hash_map<
    absl::uint128, absl::Span<const Letter>>::const_iterator BlankIterator;
typedef absl::flat_hash_map<absl::uint128,
                            absl::Span<const LetterPair>>::const_iterator
    DoubleBlankIterator;
typedef absl::variant<WordIterator, BlankIterator, DoubleBlankIterator>
    AnagramMapIterator;

class AnagramMap {
 public:
  class WordRange {
   public:
    explicit WordRange(const absl::Span<const LetterString>* span);
    explicit WordRange(const std::vector<absl::uint128>& products,
                       const AnagramMap& anagram_map);
    const std::vector<absl::Span<const LetterString>>& Spans() const {
      return spans_;
    }

   private:
    std::vector<absl::Span<const LetterString>> MakeSpans(
        const absl::Span<const LetterString>* span) const;
    std::vector<absl::Span<const LetterString>> MakeSpans(
        const std::vector<absl::uint128>& products,
        const AnagramMap& anagram_map) const;

    const std::vector<absl::Span<const LetterString>> spans_;
  };

  explicit AnagramMap(const Tiles& tiles) : tiles_(tiles) {}
  static std::unique_ptr<AnagramMap> CreateFromTextfile(
      const Tiles& tiles, const std::string& filename);
  static std::unique_ptr<AnagramMap> CreateFromBinaryFile(
      const Tiles& tiles, const std::string& filename);
  absl::Status WriteToBinaryFile(const std::string& filename) const;

  AnagramMapIterator WordIterator(const absl::uint128& product,
                                  int num_blanks) const;
  bool HasWord(const AnagramMapIterator& iterator) const;
  bool HasWord(const absl::uint128& product, int num_blanks) const;
  WordRange Words(const AnagramMapIterator& iterator,
                  const absl::uint128& product) const;
  WordRange Words(const absl::uint128& product, int num_blanks) const;
  const absl::Span<const LetterString>* Words(
      const absl::uint128& product) const;
  const absl::Span<const Letter>* Blanks(const absl::uint128& product) const;
  const absl::Span<const LetterPair>* DoubleBlanks(
      const absl::uint128& product) const;

  const uint32_t Hooks(const LetterString& letter_string) const {
    auto it = hook_map_.find(letter_string);
    if (it == hook_map_.end()) {
      return 0;
    }
    return it->second;
  }

 private:
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile);
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile2);
  FRIEND_TEST(AnagramMapTest, WriteToOstream);
  FRIEND_TEST(AnagramMapTest, CreateFromBinaryFile);
  FRIEND_TEST(AnagramMapTest, CreateFromBinaryFile2);

  void BuildHookMap();

  bool WriteToOstream(std::ostream& os) const;

  const Tiles& tiles_;
  absl::flat_hash_map<absl::uint128, absl::Span<const LetterString>> map_;
  absl::flat_hash_map<absl::uint128, absl::Span<const Letter>> blank_map_;
  absl::flat_hash_map<absl::uint128, absl::Span<const LetterPair>>
      double_blank_map_;
  std::vector<LetterString> words_;
  absl::flat_hash_map<absl::uint128, uint32_t> word_indices_;
  std::vector<Letter> blanks_;
  absl::flat_hash_map<absl::uint128, uint32_t> blank_indices_;
  std::vector<LetterPair> double_blanks_;
  absl::flat_hash_map<absl::uint128, uint32_t> double_blank_indices_;

  // Keys are LetterStrings with one empty character which can be filled with a
  // letter to make a legal word.
  absl::flat_hash_map<LetterString, uint32_t> hook_map_;
};

inline AnagramMap::WordRange::WordRange(
    const absl::Span<const LetterString>* span)
    : spans_(MakeSpans(span)) {}
inline AnagramMap::WordRange::WordRange(
    const std::vector<absl::uint128>& products, const AnagramMap& anagram_map)
    : spans_(MakeSpans(products, anagram_map)) {}

inline std::vector<absl::Span<const LetterString>>
AnagramMap::WordRange::MakeSpans(
    const absl::Span<const LetterString>* span) const {
  if (span == nullptr) {
    return {};
  } else {
    return {*span};
  }
}

inline AnagramMapIterator AnagramMap::WordIterator(const absl::uint128& product,
                                                   int num_blanks) const {
  switch (num_blanks) {
    case 0:
      return map_.find(product);
    case 1:
      return blank_map_.find(product);
    case 2:
      return double_blank_map_.find(product);
    default:
      LOG(INFO) << "Invalid number of blanks: " << num_blanks;
      return map_.end();
  }
}

inline bool AnagramMap::HasWord(const absl::uint128& product,
                                int num_blanks) const {
  switch (num_blanks) {
    case 0:
      return map_.find(product) != map_.end();
    case 1:
      return blank_map_.find(product) != blank_map_.end();
    case 2:
      return double_blank_map_.find(product) != double_blank_map_.end();
    default:
      LOG(INFO) << "Invalid number of blanks: " << num_blanks;
      return false;
  }
}

inline bool AnagramMap::HasWord(const AnagramMapIterator& iterator) const {
  if (auto it = absl::get_if<absl::flat_hash_map<
          absl::uint128, absl::Span<const LetterString>>::const_iterator>(
          &iterator)) {
    return *it != map_.end();
  } else if (auto it = absl::get_if<absl::flat_hash_map<
                 absl::uint128, absl::Span<const Letter>>::const_iterator>(
                 &iterator)) {
    return *it != blank_map_.end();
  } else if (auto it = absl::get_if<absl::flat_hash_map<
                 absl::uint128, absl::Span<const LetterPair>>::const_iterator>(
                 &iterator)) {
    return *it != double_blank_map_.end();
  }
  return false;
}

inline AnagramMap::WordRange AnagramMap::Words(
    const AnagramMapIterator& iterator, const absl::uint128& product) const {
  if (auto it = absl::get_if<absl::flat_hash_map<
          absl::uint128, absl::Span<const LetterString>>::const_iterator>(
          &iterator)) {
    if (*it == map_.end()) {
      return WordRange(nullptr);
    } else {
      return WordRange(&((*it)->second));
    }
  } else if (auto it = absl::get_if<absl::flat_hash_map<
                 absl::uint128, absl::Span<const Letter>>::const_iterator>(
                 &iterator)) {
    if (*it == blank_map_.end()) {
      return WordRange(nullptr);
    } else {
      std::vector<absl::uint128> products;
      const auto& blanks = (*it)->second;
      products.reserve(blanks.size());
      for (const Letter letter : blanks) {
        const absl::uint128 new_product = product * tiles_.Prime(letter);
        products.emplace_back(new_product);
      }
      return WordRange(products, *this);
    }
  } else if (auto it = absl::get_if<absl::flat_hash_map<
                 absl::uint128, absl::Span<const LetterPair>>::const_iterator>(
                 &iterator)) {
    if (*it == double_blank_map_.end()) {
      return WordRange(nullptr);
    } else {
      std::vector<absl::uint128> products;
      const auto& double_blanks = (*it)->second;
      products.reserve(double_blanks.size());
      for (const auto& blank_pair : double_blanks) {
        const absl::uint128 new_product =
            product *
            (tiles_.Prime(blank_pair.first) * tiles_.Prime(blank_pair.second));
        products.emplace_back(new_product);
      }
      return WordRange(products, *this);
    }
  }
  return WordRange(nullptr);
}

inline const absl::Span<const LetterString>* AnagramMap::Words(
    const absl::uint128& product) const {
  const auto it = map_.find(product);
  if (it == map_.end()) {
    return nullptr;
  }
  return &it->second;
}

inline const absl::Span<const Letter>* AnagramMap::Blanks(
    const absl::uint128& product) const {
  const auto it = blank_map_.find(product);
  if (it == blank_map_.end()) {
    return nullptr;
  }
  return &it->second;
}

inline const absl::Span<const LetterPair>* AnagramMap::DoubleBlanks(
    const absl::uint128& product) const {
  const auto it = double_blank_map_.find(product);
  if (it == double_blank_map_.end()) {
    return nullptr;
  }
  return &it->second;
}

#endif  // SRC_ANAGRAM_ANAGRAM_MAP_H