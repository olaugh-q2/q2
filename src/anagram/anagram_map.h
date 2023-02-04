#ifndef SRC_ANAGRAM_ANAGRAM_MAP_H_
#define SRC_ANAGRAM_ANAGRAM_MAP_H_

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

typedef std::pair<Letter, Letter> LetterPair;

class AnagramMap {
 public:
  class WordRange {
   public:
    explicit WordRange(const absl::Span<const LetterString>* span)
        : spans_(MakeSpans(span)) {}
    explicit WordRange(const std::vector<absl::uint128>& products,
                       const AnagramMap& anagram_map)
        : spans_(MakeSpans(products, anagram_map)) {}
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

#endif  // SRC_ANAGRAM_ANAGRAM_MAP_H_