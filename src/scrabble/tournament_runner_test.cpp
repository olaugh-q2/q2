#include "tournament_runner.h"

#include <google/protobuf/text_format.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/alpha_beta_player.h"
#include "src/scrabble/endgame_player.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/simming_player.h"
#include "src/scrabble/specializing_player.h"
#include "src/scrabble/static_player.h"
#include "src/scrabble/tile_ordering_cache.h"
#include "src/scrabble/unseen_tiles_predicate.h"

using ::google::protobuf::Arena;

class TournamentRunnerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    PassingPlayer::Register();
    StaticPlayer::Register();
    SimmingPlayer::Register();
    EndgamePlayer::Register();
    AlphaBetaPlayer::Register();
    SpecializingPlayer::Register();
    UnseenTilesPredicate::Register();
    TileOrderingCache::Register();
  }
};
/*
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
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredStatic) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 2
        number_of_threads: 1
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
    for (const auto& a : results->player_averages()) {
    LOG(INFO) << "player_averages: " << std::endl << a.DebugString();
  }
}
*/

/*
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
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 12
        number_of_threads: 12
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          static_player_config {
            id: 1
            name: "High Score"
            nickname: "H"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/zeroes.qlv"
          }
        }
        players {
          static_player_config {
            id: 2
            name: "Static"
            nickname: "S"
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
  for (const auto& a : results->player_averages()) {
    LOG(INFO) << "player_averages: " << std::endl << a.DebugString();
  }
}
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredMacondoVsQuackle) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 2400
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          static_player_config {
            id: 1
            name: "Quackle Leaves"
            nickname: "Q"
            anagram_map_file: "src/scrabble/testdata/csw21.qam"
            board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
            tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
            leaves_file: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
          }
        }
        players {
          static_player_config {
            id: 2
            name: "Macondo Leaves"
            nickname: "M"
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
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredSpecializing) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 2
        number_of_threads: 1
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
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
*/

TEST_F(TournamentRunnerTest, HeadsUpMirroredSpecializingMvQ) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        singleton_components {
          tile_ordering_provider_config {
            tile_ordering_cache_config {
              tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
              num_random_exchange_dividends: 40
            }  
          }
        }
        number_of_rounds: 240
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_quackle.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
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

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredQuickEndgame) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }

          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 2
        number_of_threads: 12
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + Quick Endgame"
            nickname: "MQuick"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                endgame_player_config {
                  id: 102
                  name: "Quick Endgame"
                  nickname: "Quick"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv" plies: 0 leave_score_weight: 1.0
                  leave_value_weight: 0.5
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
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
                  id: 201
                  name: "Macondo Leaves"
                  nickname: "M"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
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
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredLeaveWeights) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }

          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 24
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + LeaveWeightA"
            nickname: "MLWA"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                endgame_player_config {
                  id: 102
                  name: "LeaveWeightA"
                  nickname: "Quick"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv" plies: 0 leave_score_weight: 1.70
                  leave_value_weight: 0.00
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
            name: "Macondo + LeaveWeightB"
            nickname: "MLWB"
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
                  name: "Macondo Leaves"
                  nickname: "M"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                endgame_player_config {
                  id: 102
                  name: "LeaveWeightB"
                  nickname: "Quick"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv" plies: 0 leave_score_weight: 1.70
                  leave_value_weight: 0.30
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

TEST_F(TournamentRunnerTest, HeadsUpMirroredCapsPerPly) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }

          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 240
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + LeaveWeightA"
            nickname: "MLWA"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                endgame_player_config {
                  id: 102
                  name: "LeaveWeightA"
                  nickname: "Quick"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
                  plies: 1
                  leave_score_weight: 1.70
                  leave_value_weight: 0.30
                  caps_per_ply: 800
                  caps_per_ply: 400
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
            name: "Macondo + LeaveWeightB"
            nickname: "MLWB"
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
                  name: "Macondo Leaves"
                  nickname: "M"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                endgame_player_config {
                  id: 102
                  name: "LeaveWeightB"
                  nickname: "Quick"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
                  plies: 1 
                  leave_score_weight: 1.70
                  leave_value_weight: 0.30
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
*/
/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredAlphaBeta) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 100000
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
            name: "Macondo + AlphaBeta"
            nickname: "MAB"
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
                  name: "Macondo Leaves"
                  nickname: "M"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                alpha_beta_player_config {
                  id: 202
                  name: "Alpha Beta"
                  nickname: "AB"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
                  plies: 20
                  detect_stuck_tiles: true
                  unstuck_leave_score_weight: 0.6
                  unstuck_leave_value_weight: 0.08
                  caps_per_ply: 4
                  caps_per_ply: 2 
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
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpStuckTiles) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        number_of_rounds: 5000000
        number_of_threads: 24
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Macondo + NoStuck"
            nickname: "MNSt"
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
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                alpha_beta_player_config {
                  id: 102
                  name: "Stuck1"
                  nickname: "AB"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
                  plies: 20
                  detect_stuck_tiles: true
                  stuck_tiles_left_multiplier: 0.2
                  stuck_leave_score_multiplier: 1.0
                  stuck_leave_value_multiplier: 0.2
                  opp_stuck_score_multiplier: 1.6
                  unstuck_leave_score_weight: 0.6
                  unstuck_leave_value_weight: 0.08
                  caps_per_ply: 3
                  caps_per_ply: 2
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
            name: "Macondo + AlphaBetaStuck"
            nickname: "MAB"
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
                  name: "Macondo Leaves"
                  nickname: "M"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
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
                alpha_beta_player_config {
                  id: 202
                  name: "Stuck2"
                  nickname: "AB"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
                  plies: 20
                  detect_stuck_tiles: true
                  stuck_tiles_left_multiplier: 0.2
                  stuck_leave_score_multiplier: 1.0
                  stuck_leave_value_multiplier: 0.2
                  opp_stuck_score_multiplier: 2.0
                  unstuck_leave_score_weight: 0.6
                  unstuck_leave_value_weight: 0.08
                  caps_per_ply: 3
                  caps_per_ply: 2
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
*/

/*
TEST_F(TournamentRunnerTest, HeadsUpMirroredSimming) {
  Arena arena;
  auto spec = Arena::CreateMessage<q2::proto::TournamentSpec>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        data_collection {
          tiles_files: "src/scrabble/testdata/english_scrabble_tiles.textproto"
          board_files: "src/scrabble/testdata/scrabble_board.textproto"
          anagram_map_file_specs {
            anagram_map_filename: "src/scrabble/testdata/csw21.qam"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/csw_scrabble_quackle.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
          leaves_file_specs {
            leaves_filename: "src/scrabble/testdata/zeroes.qlv"
            tiles_filename:
"src/scrabble/testdata/english_scrabble_tiles.textproto"
          }
        }
        singleton_components {
          tile_ordering_provider_config {
            tile_ordering_cache_config {
              tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
              num_random_exchange_dividends: 40
            }  
          }
        }
        number_of_rounds: 4000
        number_of_threads: 16
        format: HEADS_UP_MIRRORED_PAIRS
        players {
          specializing_player_config {
            id: 1
            name: "Simming+Score"
            nickname: "MHSE"
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 36
                  max_unseen_tiles: 10000
                }
              }
              player {
                simming_player_config {
                  id: 101
                  name: "Simming"
                  nickname: "S"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
                  plies: 4
                  max_plays_considered: 2
                  static_equity_pruning_threshold: 5
                  min_iterations: 5000
                  max_iterations: 5000
                  rollout_player {
                    static_player_config {
                      id: 1001
                      name: "Rollout Static Player"
                      nickname: "RollStat"
                      anagram_map_file: "src/scrabble/testdata/csw21.qam"
                      board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
                      tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
                      leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
                    }
                  }
                }
              }
            }
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 0
                  max_unseen_tiles: 35
                }
              }
              player {
                static_player_config {
                  id: 102
                  name: "High Score"
                  nickname: "Score"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
                }
              }
            }
          }
        }
        players {
          specializing_player_config {
            id: 2
            name: "Macondo+Score"
            nickname: "M+S"
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 36
                  max_unseen_tiles: 10000
                }
              }
              player {
                static_player_config {
                  id: 201
                  name: "Macondo"
                  nickname: "Macondo"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/csw_scrabble_macondo.qlv"
                }
              }
            }
            conditional_players {
              predicate {
                unseen_tiles_predicate_config {
                  min_unseen_tiles: 0
                  max_unseen_tiles: 35
                }
              }
              player {
                static_player_config {
                  id: 202
                  name: "Score 2"
                  nickname: "HSE"
                  anagram_map_file: "src/scrabble/testdata/csw21.qam"
                  board_layout_file:
"src/scrabble/testdata/scrabble_board.textproto" tiles_file:
"src/scrabble/testdata/english_scrabble_tiles.textproto" leaves_file:
"src/scrabble/testdata/zeroes.qlv"
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
*/