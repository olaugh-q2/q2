#include "src/scrabble/specializing_player.h"

#include <google/protobuf/text_format.h>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/specializing_player.h"
#include "src/scrabble/static_player.h"
#include "src/scrabble/unseen_tiles_predicate.h"

using ::google::protobuf::Arena;

class SpecializingPlayerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    PassingPlayer::Register();
    SpecializingPlayer::Register();
    StaticPlayer::Register();
    UnseenTilesPredicate::Register();
  }
};

TEST_F(SpecializingPlayerTest, CreateSpecializingPlayer) {
  DataManager* dm = DataManager::GetInstance();
  EXPECT_NE(dm, nullptr);
  Arena arena;
  auto data = Arena::CreateMessage<q2::proto::DataCollection>(&arena);
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
                                                data);
  dm->LoadData(*data);

  auto config =
      Arena::CreateMessage<q2::proto::SpecializingPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "Specializing Player"
        nickname: "Speccy"
        conditional_players {
            predicate {
                unseen_tiles_predicate_config {
                    min_unseen_tiles: 8
                    max_unseen_tiles: 10000
                }
            }
            player {
                static_player_config {
                    id: 100
                    name: "Speccy's Static Player"
                    nickname: "SpecStat"
                    anagram_map_file: "src/scrabble/testdata/csw21.qam"
                    board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                    tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                    leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
                }
            }
        }
        conditional_players {
            predicate {
                unseen_tiles_predicate_config {
                    min_unseen_tiles: 0
                    max_unseen_tiles: 7
                }
            }
            player {
                passing_player_config {
                    id: 100
                    name: "Speccy's Passing Player"
                    nickname: "SpecPass"
                }
            }
        }        
    )",
                                                config);
  std::unique_ptr<ComputerPlayer> player =
      ComponentFactory::GetInstance()->CreateComputerPlayer(*config);
  EXPECT_NE(player, nullptr);
  // EXPECT_TRUE(false);
}