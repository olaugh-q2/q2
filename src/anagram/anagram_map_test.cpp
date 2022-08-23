#include "src/anagram/anagram_map.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ElementsAre;
using testing::Pair;
using testing::UnorderedElementsAre;

class AnagramMapTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJ"
        "KLLLLMMMMMMMMNNNNNNOOOOOOOOPPQRRRRRRRRSSSS"
        "TTTTTTUUUUVVWWXYYZ??");
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }

  absl::uint128 P(const std::string& s) { return tiles_->ToProduct(LS(s)); }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(AnagramMapTest, CreateFromTextfile) {
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_THAT(
      anagram_map->map_,
      UnorderedElementsAre(Pair(P("AH"), ElementsAre(LS("AH"), LS("HA"))),
                           Pair(P("AHM"), ElementsAre(LS("HAM")))));

  EXPECT_THAT(anagram_map->blank_map_,
              UnorderedElementsAre(Pair(P("A"), ElementsAre(L('H'))),
                                   Pair(P("H"), ElementsAre(L('A'))),
                                   Pair(P("AH"), ElementsAre(L('M'))),
                                   Pair(P("AM"), ElementsAre(L('H'))),
                                   Pair(P("HM"), ElementsAre(L('A')))));
}

TEST_F(AnagramMapTest, CreateFromTextfile2) {
  const std::string input_filepath = "src/anagram/testdata/csw21nines.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_EQ(anagram_map->map_.size(), 39317);
  const auto words = anagram_map->Words(P("AEIOUCNRT"));
  ASSERT_NE(words, nullptr);
  EXPECT_THAT(*words, ElementsAre(LS("AUTOCRINE"), LS("CAUTIONER"),
                                  LS("COINTREAU"), LS("RECAUTION")));

  const auto blanks = anagram_map->Blanks(P("TAILERON"));
  EXPECT_THAT(*blanks, ElementsAre(L('A'),  // ALIENATOR, RATIONALE
                                   L('C'),  // CLARIONET, CROTALINE
                                   L('D'),  // RODENTIAL
                                   L('F'),  // REFLATION
                                   L('M'),  // MENTORIAL
                                   L('N'),  // INTERLOAN
                                   L('P'),  // PRELATION, RANTIPOLE
                                   L('S'),  // AEIOLNRST (x5)
                                   L('T'),  // NATROLITE, TENTORIAL
                                   L('U')   // OUTLINEAR
  ));
}
