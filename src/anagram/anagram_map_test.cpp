#include "anagram_map.h"

#include "gtest/gtest.h"

TEST(AnagramMapTest, CreateFromTextfile) {
    const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
    auto anagram_map = AnagramMap::CreateFromTextfile(input_filepath);
    ASSERT_NE(anagram_map, nullptr);
}