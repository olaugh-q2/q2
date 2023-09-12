#include "src/scrabble/simming_player.h"

#include <google/protobuf/text_format.h>

using ::google::protobuf::Arena;

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/static_player.h"
#include "src/scrabble/tile_ordering_cache.h"

class SimmingPlayerTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    StaticPlayer::Register();
    TileOrderingCache::Register();
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

    ComponentFactory* cf = ComponentFactory::GetInstance();
    auto singletons = Arena::CreateMessage<q2::proto::SingletonComponents>(&arena);
    google::protobuf::TextFormat::ParseFromString(R"(
      tile_ordering_provider_config {
          tile_ordering_cache_config {
              tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
              num_random_exchange_dividends: 40
          }  
      }
    )",
                                                  singletons);
    cf->CreateSingletonComponents(*singletons);                                                  
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
        name: "Simmie"
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
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
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
        name: "Simmie"
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
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  EXPECT_EQ(pruned_moves.size(), 57);
}

TEST_F(SimmingPlayerTest, NoPrune) {
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
        static_equity_pruning_threshold: 100000
        min_iterations: 100
        max_iterations: 100
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  EXPECT_EQ(pruned_moves.size(), 57);
}

TEST_F(SimmingPlayerTest, SelectWithinThreshold) {
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
        static_equity_pruning_threshold: 0.1
        min_iterations: 100
        max_iterations: 100
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  // Only the 80-point play is within the threshold.
  EXPECT_EQ(pruned_moves.size(), 1);
}

TEST_F(SimmingPlayerTest, SelectWithinThreshold2) {
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
        static_equity_pruning_threshold: 30
        min_iterations: 100
        max_iterations: 100
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  // The seven bingo placements are within the threshold
  EXPECT_EQ(pruned_moves.size(), 7);
}

TEST_F(SimmingPlayerTest, SelectTopNWithinThreshold) {
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
        max_plays_considered: 10
        static_equity_pruning_threshold: 30
        min_iterations: 100
        max_iterations: 100
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  // The seven bingo placements are within the threshold
  EXPECT_EQ(pruned_moves.size(), 7);
}

TEST_F(SimmingPlayerTest, SelectTopNWithinThreshold2) {
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
        max_plays_considered: 10
        static_equity_pruning_threshold: 300
        min_iterations: 100
        max_iterations: 100
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }        
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
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto all_moves = player->FindMoves(&previous_positions, *pos);
  const auto pruned_moves = player->InitialPrune(all_moves);
  // The seven bingo placements are within the threshold
  EXPECT_EQ(pruned_moves.size(), 10);
}

TEST_F(SimmingPlayerTest, ChooseBestMove) {
    Arena arena;
  auto config = Arena::CreateMessage<q2::proto::SimmingPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "Simmie"
        nickname: "S"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 2
        max_plays_considered: 9
        static_equity_pruning_threshold: 100000
        min_iterations: 1000
        max_iterations: 1000
        rollout_player {
            static_player_config {
                id: 100
                name: "Simmie's Static Player"
                nickname: "SimStat"
                anagram_map_file: "src/scrabble/testdata/csw21.qam"
                board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            }
        }
        )",
                                                config);
  auto player = absl::make_unique<SimmingPlayer>(*config);
  const Board board;
  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  const Rack rack(tiles->ToLetterString("GOULASH").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 0, 0, 0, 0, absl::Minutes(25), 0, *tiles);
  std::vector<GamePosition> previous_positions;
  const auto move = player->ChooseBestMove(&previous_positions, *pos);
  ExpectMove(move, "8F GOULASH (score = 80)");
}