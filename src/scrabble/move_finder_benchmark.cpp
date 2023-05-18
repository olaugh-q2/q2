#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/scrabble/move_finder.h"

ABSL_FLAG(std::string, rack_letters, "AERST??", "Rack letters");
ABSL_FLAG(int, num_games, 1000, "Number of games to play");

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  LOG(INFO) << "Hello world!" << std::endl;

  auto tiles = absl::make_unique<Tiles>(
      "src/scrabble/testdata/english_scrabble_tiles.textproto");
  LOG(INFO) << "tiles ok";
  auto anagram_map = AnagramMap::CreateFromBinaryFile(
      *tiles, "src/scrabble/testdata/csw21.qam");
  LOG(INFO) << "anagram_map ok";
  auto board_layout = absl::make_unique<BoardLayout>(
      "src/scrabble/testdata/scrabble_board.textproto");
  LOG(INFO) << "board layout ok";
  auto leaves = Leaves::CreateFromBinaryFile(
      *tiles, "src/scrabble/testdata/csw_scrabble_macondo.qlv");
  LOG(INFO) << "leaves ok";
  auto move_finder = absl::make_unique<MoveFinder>(*anagram_map, *board_layout,
                                                   *tiles, *leaves);
  LOG(INFO) << "move finder ok";

  const Rack rack(
      tiles->ToLetterString(absl::GetFlag(FLAGS_rack_letters)).value());
  const int num_games = absl::GetFlag(FLAGS_num_games);
  const std::vector<Letter> empty;
  const auto empty_bag = absl::make_unique<Bag>(*tiles, empty);

  int total_moves = 0;
  LOG(INFO) << "starting games... (" << num_games << ")";
  for (int i = 0; i < num_games; ++i) {
    Board board;
    move_finder->ClearHookTables();
    for (int j = 0;; ++j) {
      move_finder->FindMoves(
          rack, board, *empty_bag, MoveFinder::RecordBest, false);
      const auto& moves = move_finder->Moves();
      if (moves[0].GetAction() != Move::Place) {
        break;
      }
      board.UnsafePlaceMove(moves[0]);
      move_finder->CacheCrossesAndScores(board, moves[0]);
      total_moves++;
    }
  }
  LOG(INFO) << "total_moves: " << total_moves;

  return 0;
}