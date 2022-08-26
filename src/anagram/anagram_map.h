#ifndef SRC_ANAGRAM_ANAGRAM_MAP_H_
#define SRC_ANAGRAM_ANAGRAM_MAP_H_

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
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
  WordRange Words(const absl::uint128& product, int num_blanks) const;
  const absl::Span<const LetterString>* Words(
      const absl::uint128& product) const;
  const absl::Span<const Letter>* Blanks(const absl::uint128& product) const;
  const absl::Span<const LetterPair>* DoubleBlanks(
      const absl::uint128& product) const;

 private:
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile);
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile2);

  const Tiles& tiles_;
  absl::flat_hash_map<absl::uint128, absl::Span<const LetterString>> map_;
  absl::flat_hash_map<absl::uint128, absl::Span<const Letter>> blank_map_;
  absl::flat_hash_map<absl::uint128, absl::Span<const LetterPair>>
      double_blank_map_;
  std::vector<LetterString> words_;
  std::vector<Letter> blanks_;
  std::vector<LetterPair> double_blanks_;
};

#endif  // SRC_ANAGRAM_ANAGRAM_MAP_H_