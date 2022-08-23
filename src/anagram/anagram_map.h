#ifndef SRC_ANAGRAM_ANAGRAM_MAP_H_
#define SRC_ANAGRAM_ANAGRAM_MAP_H_

#include "absl/container/flat_hash_map.h"
#include "absl/numeric/int128.h"
#include "absl/types/span.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class AnagramMap {
 public:
  static std::unique_ptr<AnagramMap> CreateFromTextfile(
      const Tiles& tiles, const std::string& filename);
  const absl::Span<const LetterString>* Words(
      const absl::uint128& product) const;
  const absl::Span<const Letter>* Blanks(
      const absl::uint128& product) const;   

 private:
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile);
  FRIEND_TEST(AnagramMapTest, CreateFromTextfile2);

  absl::flat_hash_map<absl::uint128, absl::Span<const LetterString>> map_;
  absl::flat_hash_map<absl::uint128, absl::Span<const Letter>> blank_map_;
  std::vector<LetterString> words_;
  std::vector<Letter> blanks_;
};

#endif  // SRC_ANAGRAM_ANAGRAM_MAP_H_