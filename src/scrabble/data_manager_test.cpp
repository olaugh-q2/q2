#include "src/scrabble/data_manager.h"

#include <google/protobuf/text_format.h>

#include "gtest/gtest.h"

using ::google::protobuf::Arena;

TEST(DataManagerTest, Test) {
  DataManager* dm = DataManager::GetInstance();
  EXPECT_NE(dm, nullptr);
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::DataCollection>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
    tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    board_files: "src/scrabble/testdata/scrabble_board.textproto"
    anagram_map_file_specs {
        anagram_map_filename: "src/scrabble/testdata/csw21.qam"
        tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    }
    leaves_file_specs {
        leaves_filename: "src/scrabble/testdata/english_leaves.textproto"
        tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
    }
  )",
                                                spec);
  dm->LoadData(*spec);

  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);
  EXPECT_EQ(tiles->ToProduct(tiles->ToLetterString("EEE").value()), 2 * 2 * 2);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  EXPECT_NE(layout, nullptr);
  std::stringstream ss;
  layout->DisplayHeader(ss);
  EXPECT_EQ(ss.str(), "  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ");

  const AnagramMap* anagram_map =
      dm->GetAnagramMap("src/scrabble/testdata/csw21.qam");
  EXPECT_NE(anagram_map, nullptr);
  auto qi = anagram_map->WordIterator(
      tiles->ToProduct(tiles->ToLetterString("QI").value()), 0);
  EXPECT_TRUE(anagram_map->HasWord(qi));
  auto xx = anagram_map->WordIterator(
      tiles->ToProduct(tiles->ToLetterString("XX").value()), 0);
  EXPECT_FALSE(anagram_map->HasWord(xx));
}
