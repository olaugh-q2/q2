#include "src/scrabble/endgame_player.h"

#include <google/protobuf/text_format.h>

using ::google::protobuf::Arena;

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EndgamePlayerTest : public ::testing::Test {
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

TEST_F(EndgamePlayerTest, ZeroPlyGreedyPlayout) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::EndgamePlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "Endgame"
        nickname: "E"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        check_altered_plays: true
        )",
                                                config);
  auto player = absl::make_unique<EndgamePlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O   -> olaugh     IIDGLMN   395)",
       R"(   ------------------------------     cwac       ADINRSY   448)",
       R"( 1|P     '       F E R N L E s S| --Speedy Player's choices---)",
       R"( 2|A -       B O U N "       -  | best  12L MING   26 IDL)",
       R"( 3|T W O C K E R S '   J   -    |  4.00 12L MILD   26 IGN)",
       R"( 4|I     O E     C     O F     '|  5.00 12L DING   22 ILM)",
       R"( 5|O     L A           T O      |  8.00 12L MIND   26 IGL)",
       R"( 6|  "   L   " Q     P A H   "  |  13.0 12L LING   18 IDM)",
       R"( 7|    ' I     A   ' H   N '    |  27.0 12L DIG    16 ILMN)",
       R"( 8|I G A D     T A X I   S O W M|  29.0 12L LIND   18 IGM)",
       R"( 9|  A Y E     ' T I Z     '    |  39.0 O7  M(M)   6  IIDGLN)",
       R"(10|  N E R V U R E   "       "  |  40.0 O8  (M)M   6  IIDGLN)",
       R"(11|        - R E s T I V E      |  46.0 O6  LI(M)N 6  IDGM)",
       R"(12|B O U T A D E '       -     '| --Tracking------------------)",
       R"(13|E U G E     '   '       -    | ADINRSY  7)",
       R"(14|  -       "       "       -  |)",
       R"(15|=     '       =       '     =|)",
       R"(   ------------------------------)"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("MILDING").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 448, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, "12L MING (score = 26)");
}