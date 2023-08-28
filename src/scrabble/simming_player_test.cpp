#include "src/scrabble/simming_player.h"

#include <google/protobuf/text_format.h>

using ::google::protobuf::Arena;

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SimmingPlayerTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    DataManager* dm = DataManager::GetInstance();
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
          leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
      }
    )",
                                                  spec);
    dm->LoadData(*spec);
  }

  void ExpectMove(const Move& move, const std::string& expected) {
    const Tiles* tiles = DataManager::GetInstance()->GetTiles(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    std::stringstream ss;
    move.Display(*tiles, ss);
    const auto actual = ss.str();
    EXPECT_EQ(actual, expected);
  }
};

TEST_F(SimmingPlayerTest, SelectTopN) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::SimmingPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "Simming"
        nickname: "S"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 1
        max_plays_considered: 5
        static_equity_pruning_threshold: 100000
        min_iterations: 100
        max_iterations: 100
        )",
                                                config);
  auto player = absl::make_unique<SimmingPlayer>(*config);
  const Board board;
  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  const Rack rack(tiles->ToLetterString("OLAUGHS").value());
  auto pos = absl::make_unique<GamePosition>(*layout, board, 1, 2, rack, 0, 0,
                                             0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  EXPECT_EQ(pruned_moves.size(), 5);
}

TEST_F(SimmingPlayerTest, UnneededSelectTopN) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::SimmingPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "Simming"
        nickname: "S"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 1
        max_plays_considered: 200
        static_equity_pruning_threshold: 100000
        min_iterations: 100
        max_iterations: 100
        )",
                                                config);
  auto player = absl::make_unique<SimmingPlayer>(*config);
  const Board board;
  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  const Rack rack(tiles->ToLetterString("VIVIFIC").value());
  auto pos = absl::make_unique<GamePosition>(*layout, board, 1, 2, rack, 0, 0,
                                             0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  EXPECT_EQ(pruned_moves.size(), 57);
}