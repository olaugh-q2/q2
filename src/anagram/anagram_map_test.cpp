#include "src/anagram/anagram_map.h"

#include "absl/memory/memory.h"
#include "gtest/gtest.h"

class AnagramMapTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJ"
        "KLLLLMMMMMMMMNNNNNNOOOOOOOOPPQRRRRRRRRSSSS"
        "TTTTTTUUUUVVWWXYYZ??");
  }
  std::unique_ptr<Tiles> tiles_;
};

TEST_F(AnagramMapTest, CreateFromTextfile) {
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
}