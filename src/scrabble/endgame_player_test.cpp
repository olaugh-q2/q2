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

  void ExpectMoveAmong(const Move& move,
                       const std::vector<std::string>& expected) {
    const Tiles* tiles = DataManager::GetInstance()->GetTiles(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    std::stringstream ss;
    move.Display(*tiles, ss);
    const auto actual = ss.str();
    EXPECT_THAT(expected, ::testing::Contains(actual));
  }
};

TEST_F(EndgamePlayerTest, Ming) {
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
        plies: 1
        leave_score_weight: 1.7
        leave_value_weight: 0.3
        caps_per_ply: 300
        caps_per_ply: 300
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

TEST_F(EndgamePlayerTest, Alkyd) {
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
        plies: 1
        leave_score_weight: 1.7
        leave_value_weight: 0.3   
        caps_per_ply: 300
        caps_per_ply: 300
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
    {R"(   A B C D E F G H I J K L M N O      olaugh    AEOR      471)",
     R"(   ------------------------------  -> cwac      AADDILW   384)",
     R"( 1|=     '     M E N T U M     =| --Speedy Player's choices--)",
     R"( 2|S E N S A T E   E "       -  | best *11B AL(KY)D 13 ADIW  )",
     R"( 3|U   -       '   R O A T E    |  3.00 12A ALI(YA) 9  ADDW  )",
     R"( 4|T     -       V E R G E D   '|  7.00 11B AL(KY)  11 ADDIW )",
     R"( 5|O       -       I   -        |  12.0 2L  AWDL    29 ADI   )",
     R"( 6|R "   L O P     D "       " H|  15.0 3E  DAW     28 ADIL  )",
     R"( 7|I   '   B U F O S       ' J A|  16.0 F14 AW      13 ADDIL )",
     R"( 8|a     Z I N E B     L O G O I|  17.0 13E (W)AWL  14 ADDI  )",
     R"( 9|N   ' I     H A V I O R '   R|  17.0 13E (W)AWA  14 DDIL  )",
     R"(10|  "   N   "       F U G   " T|  17.0 13E (W)AW   13 ADDIL )",
     R"(11|      K Y           -     E A|  18.0 B8  DAWD    21 AIL   )",
     R"(12|'     Y A     '       -   X I| --Tracking-----------------)",
     R"(13|    -   W   '   '       - E L| AEOR  4                    )",
     R"(14|  -     P "       Q       C  |                            )",
     R"(15|=     '       p E I N C T S =|                            )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("AADDILW").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 471, 384, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, "11B AL..D (score = 13)");
}

TEST_F(EndgamePlayerTest, ViceOrVirge) {
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
        plies: 1
        leave_score_weight: 1.7
        leave_value_weight: 0.3
        caps_per_ply: 300
        caps_per_ply: 300
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
      {R"(   A B C D E F G H I J K L M N O   -> olaugh    EEIIDRV   345)",
       R"(   ------------------------------     cwac      IOOSTTX   446)",
       R"( 1|E     J A V E L       '   G =| --Speedy Player's choices--)",
       R"( 2|U - M O   "       "       L  | best *4L  VI(C)E   20 EIDR )",
       R"( 3|O N E R     '   '       - Y  | best  1K  VIR(G)E  30 EID  )",
       R"( 4|I O S -       '       -   C '|  4.00 F4  IDE      13 EIRV )",
       R"( 5|  U T   I           -     E  |  5.00 4J  DEVI(C)E 26 IR   )",
       R"( 6|  P i   N "       "       R  |  5.00 1K  RID(G)E  24 EIV  )",
       R"( 7|    Z   T   F A B       ' I  |  5.00 1K  DIR(G)E  24 EIV  )",
       R"( 8|=   A G O R A E     D A N C Y|  5.00 1J  DIRI(G)E 27 EV   )",
       R"( 9|    S   W E D   Q U O D '    |  31.0 D6  RI(G)    21 EEIDV)",
       R"(10|  "   A N "     I N R O   "  |  38.0 3L  IV(Y)    18 EEIDR)",
       R"(11|      M -         R -        |  40.0 12H VE(E)    15 EIIDR)",
       R"(12|'     P       '   E   A H I '| --Tracking-----------------)",
       R"(13|    - U     '   W E F T E    | IOOSTTX  7                 )",
       R"(14|  T A L A K       L       -  |                            )",
       R"(15|=   B E N I G H t S   '     =|                            )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("EEIIDRV").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 345, 446, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMoveAmong(move, {"4L VI.E (score = 20)", "1K VIR.E (score = 30)"});
}