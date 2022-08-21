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

  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }

  absl::uint128 Prod(const std::string& s) { return tiles_->ToProduct(LS(s)); }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(AnagramMapTest, CreateFromTextfile) {
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_THAT(
      anagram_map->map_,
      UnorderedElementsAre(Pair(Prod("AH"), ElementsAre(LS("AH"), LS("HA"))),
                           Pair(Prod("AHM"), ElementsAre(LS("HAM")))));
}

TEST_F(AnagramMapTest, CreateFromTextfile2) {
  const std::string input_filepath = "src/anagram/testdata/csw21.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_EQ(anagram_map->map_.size(), 247168);
}
