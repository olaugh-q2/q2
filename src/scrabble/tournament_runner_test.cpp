#include "tournament_runner.h"

#include <google/protobuf/text_format.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/specializing_player.h"
#include "src/scrabble/static_player.h"
#include "src/scrabble/unseen_tiles_predicate.h"

using ::google::protobuf::Arena;

class TournamentRunnerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    PassingPlayer::Register();
    StaticPlayer::Register();
    SpecializingPlayer::Register();
    UnseenTilesPredicate::Register();
  }
};

TEST_F(TournamentRunnerTest, HeadsUpMirroredPassing) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
        }
        number_of_rounds: 2
        number_of_threads: 1
        players {
          passing_player_config {
            id: 1
            name: "Passer 1"
            nickname: "P1"
          }
        }
        players {
          passing_player_config {
            id: 2
            name: "Passer 2"
            nickname: "P2"
          }
        }
    )",
                                                spec);
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
}

TEST_F(TournamentRunnerTest, HeadsUpMirroredStatic) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
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
        }
        number_of_rounds: 240
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          static_player_config {
            id: 1
            name: "Static 1"
            nickname: "S1"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          }
        }
        players {
          static_player_config {
            id: 2
            name: "Static 2"
            nickname: "S2"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          }
        }
    )",
                                                spec);
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
}

TEST_F(TournamentRunnerTest, HeadsUpMirroredStaticVsScore) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
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
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 24
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          static_player_config {
            id: 1
            name: "Static"
            nickname: "S"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          }
        }
        players {
          static_player_config {
            id: 2
            name: "High Score"
            nickname: "H"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/zeroes.qlv"
          }
        }
    )",
                                                spec);
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
}

TEST_F(TournamentRunnerTest, HeadsUpMirroredMacondoVsQuackle) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
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
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 24
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          static_player_config {
            id: 1
            name: "Macondo Leaves"
            nickname: "M"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          }
        }
        players {
          static_player_config {
            id: 2
            name: "Quackle Leaves"
            nickname: "Q"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
          }
        }
    )",
                                                spec);
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
}

TEST_F(TournamentRunnerTest, HeadsUpMirroredSpecializing) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
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
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 24
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + High Score Endgame"
            nickname: "MHSE"
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 8
                  max_unseen_tiles: 10000
                }
              }
              player {
                static_player_config {
                  id: 101
                  name: "Macondo Leaves"
                  nickname: "M"
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
                static_player_config {
                  id: 102
                  name: "High Score Endgame"
                  nickname: "HSE"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                  tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                  leaves_file: "src/scrabble/testdata/zeroes.qlv"
                }
              }
            }
          }            
        }
        players {
          static_player_config {
            id: 2
            name: "All Macondo"
            nickname: "Macondo"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
          }
        }
    )",
                                                spec);                                                
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
  for (const auto& a : results->player_averages()) {
    LOG(INFO) << "player_averages: " << std::endl << a.DebugString();
  }
}

TEST_F(TournamentRunnerTest, HeadsUpMirroredSpecializingMvQ) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
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
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 96
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + High Score Endgame"
            nickname: "MHSE"
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 8
                  max_unseen_tiles: 10000
                }
              }
              player {
                static_player_config {
                  id: 101
                  name: "Macondo Leaves"
                  nickname: "M"
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
                static_player_config {
                  id: 102
                  name: "High Score Endgame"
                  nickname: "HSE"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                  tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                  leaves_file: "src/scrabble/testdata/zeroes.qlv"
                }
              }
            }
          }            
        }
        players {
          specializing_player_config {
            id: 2
            name: "Quackle + High Score Endgame"
            nickname: "QHSE"
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 8
                  max_unseen_tiles: 10000
                }
              }
              player {
                static_player_config {
                  id: 201
                  name: "Quackle Leaves"
                  nickname: "Q"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                  tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                  leaves_file: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
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
                static_player_config {
                  id: 202
                  name: "High Score Endgame"
                  nickname: "HSE"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                  tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                  leaves_file: "src/scrabble/testdata/zeroes.qlv"
                }
              }
            }
          }            
        }
    )",
                                                spec);                                                
  TournamentRunner runner(*spec);
  auto results = Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  runner.Run(results);
  for (const auto& a : results->player_averages()) {
    LOG(INFO) << "player_averages: " << std::endl << a.DebugString();
  }
}