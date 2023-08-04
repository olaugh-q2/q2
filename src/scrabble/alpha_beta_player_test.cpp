#include "src/scrabble/alpha_beta_player.h"

#include <google/protobuf/text_format.h>

using ::google::protobuf::Arena;

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class AlphaBetaPlayerTest : public ::testing::Test {
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

TEST_F(AlphaBetaPlayerTest, Ming) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.4
        unstuck_leave_value_weight: 0.1
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 50
        caps_per_ply: 2        
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
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

TEST_F(AlphaBetaPlayerTest, Alkyd) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 300
        caps_per_ply: 300
        caps_per_ply: 300
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
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

TEST_F(AlphaBetaPlayerTest, ViceOrVirge) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.4
        unstuck_leave_value_weight: 0.1
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 50
        caps_per_ply: 2        
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
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

TEST_F(AlphaBetaPlayerTest, Gio) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 20
        caps_per_ply: 3
        caps_per_ply: 3
        caps_per_ply: 2        
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O   -> olaugh          EPILOGI   547)",
       R"(   ------------------------------     cwac            BCHSV     443)",
       R"( 1|=     '       T R A N Q     =| --Speedy Player's choices--      )",
       R"( 2|  -       "       "   A   - G| best *11M G(I)O   18 100.0% EPILI)",
       R"( 3|    -       '   '     I -   U| best  B7  GO      8  100.0% EPILI)",
       R"( 4|'     -       '     O D     T|  1.00 2J  GO(A)   17 100.0% EPILI)",
       R"( 5|    L E A N E S T   U       T|  3.00 B7  GIE     11 100.0% PLOI )",
       R"( 6|C "       O R I E N T E R " L|  5.00 I11 GO(O)   13 100.0% EPILI)",
       R"( 7|A   '       '   '   B   ' M E|  5.00 4D  OP      16 100.0% EILGI)",
       R"( 8|T     '       A N E R G I A S|  5.00 M2  GO      13 100.0% EPILI)",
       R"( 9|A   '   M O N E Y   E   ' U  |  8.00 C5  (L)OGIE 9  100.0% PLI  )",
       R"(10|R "   F A Y       Z E     V  |  9.00 B7  GOE     11 100.0% PILI )",
       R"(11|R I L E D         O d     I  |  9.00 7A  (A)GILE 7  100.0% POI  )",
       R"(12|H   O W E     K       J I N X| --Tracking-----------------------)",
       R"(13|    -       ' a O U D A D S  | BCHSV  5                         )",
       R"(14|  -       "   I   P O W   -  |                                  )",
       R"(15|=     '       F       '     =|                                  )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("EPILOGI").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 345, 446, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"11M G.O (score = 18)"});
}

TEST_F(AlphaBetaPlayerTest, Ort) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 32
        caps_per_ply: 16
        caps_per_ply: 8
        caps_per_ply: 4
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O   -> cwac             BOQ     395)",
       R"(   ------------------------------     olaugh           CIORST  459)",
       R"( 1|=     '       =       C A G Y| --Speedy Player's choices--     )",
       R"( 2|  -       "       " W O N -  | best  15A ORT    24 100.0% ICS  )",
       R"( 3|    -       '   ' S I N D    |  4.00*15A IO     15 100.0% CRST )",
       R"( 4|I     -       '   U N I     '|  4.00 N5  TORICS 24 100.0%      )",
       R"( 5|M       -         B O A      |  5.00 J13 IO(N)  8  100.0% CRST )",
       R"( 6|P "       "       V       "  |  6.00 N8  CIS    19 100.0% ORT  )",
       R"( 7|R   '       '   ' I     '    |  7.00 6L  SIC    19 100.0% ORT  )",
       R"( 8|E E K '       U N R I D     =|  9.00 15A IOS    24 100.0% CRT  )",
       R"( 9|S M I D G E O N ' A G E '    |  12.0 15E IO     11 100.0% CRST )",
       R"(10|s "       "   F I L L E T "  |  12.0 J13 SI(N)  8  100.0% OCRT )",
       R"(11|E       F A V E L   U R E    |  12.0 13G I(TA)S 8  100.0% OCRT )",
       R"(12|D     Z A X   T I     E E   '| --Tracking----------------------)",
       R"(13|  T W A Y   ' T A       T    | BOQ  3                          )",
       R"(14|L O O S   J O E   "     H U P|                                 )",
       R"(15|=     '       R A N c H E R A|                                 )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("TORICS").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 459, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"15A ORT (score = 24)"});
}

TEST_F(AlphaBetaPlayerTest, Bo) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 20
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O )",
       R"(   ------------------------------)",
       R"( 1|=     '       =       C A G Y|)",
       R"( 2|  -       "       " W O N -  |)",
       R"( 3|    -       '   ' S I N D    |)",
       R"( 4|I     -       '   U N I     '|)",
       R"( 5|M       -         B O A      |)",
       R"( 6|P "       "       V       "  |)",
       R"( 7|R   '       '   ' I     '    |)",
       R"( 8|E E K '       U N R I D     =|)",
       R"( 9|S M I D G E O N ' A G E '    |)",
       R"(10|s "       "   F I L L E T "  |)",
       R"(11|E       F A V E L   U R E    |)",
       R"(12|D     Z A X   T I     E E   '|)",
       R"(13|  T W A Y   ' T A       T    |)",
       R"(14|L O O S   J O E   "     H U P|)",
       R"(15|O R T '       R A N c H E R A|)"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("BOQ").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 459, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"15E BO (score = 13)"});
}

TEST_F(AlphaBetaPlayerTest, Fillets) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 20
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O )",
       R"(   ------------------------------)",
       R"( 1|=     '       =       C A G Y|)",
       R"( 2|  -       "       " W O N -  |)",
       R"( 3|    -       '   ' S I N D    |)",
       R"( 4|I     -       '   U N I     '|)",
       R"( 5|M       -         B O A      |)",
       R"( 6|P "       "       V       "  |)",
       R"( 7|R   '       '   ' I     '    |)",
       R"( 8|E E K '       U N R I D     =|)",
       R"( 9|S M I D G E O N ' A G E '    |)",
       R"(10|s "       "   F I L L E T "  |)",
       R"(11|E       F A V E L   U R E    |)",
       R"(12|D     Z A X   T I     E E   '|)",
       R"(13|  T W A Y   ' T A       T    |)",
       R"(14|L O O S   J O E   "     H U P|)",
       R"(15|O R T ' B O   R A N c H E R A|)"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("CIS").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 459, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"10H ......S (score = 12)"});
}

TEST_F(AlphaBetaPlayerTest, Pi) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 20
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O )",
       R"(   ------------------------------)",
       R"( 1|=     '       =       C A G Y|)",
       R"( 2|  -       "       " W O N -  |)",
       R"( 3|    -       '   ' S I N D    |)",
       R"( 4|I     -       '   U N I     '|)",
       R"( 5|M       -         B O A      |)",
       R"( 6|P "       "       V       "  |)",
       R"( 7|R   '       '   ' I     '    |)",
       R"( 8|E E K '       U N R I D     =|)",
       R"( 9|S M I D G E O N ' A G E '    |)",
       R"(10|s "       "   F I L L E T S  |)",
       R"(11|E       F A V E L   U R E    |)",
       R"(12|D     Z A X   T I     E E   '|)",
       R"(13|  T W A Y   ' T A       T    |)",
       R"(14|L O O S   J O E   "     H U P|)",
       R"(15|O R T ' B O   R A N c H E R A|)"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("CI").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 459, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"6A .I (score = 6)"});
}

TEST_F(AlphaBetaPlayerTest, Pic) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 100
        caps_per_ply: 50
        caps_per_ply: 20
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O )",
       R"(   ------------------------------)",
       R"( 1|=     '       =       C A G Y|)",
       R"( 2|  -       "       " W O N -  |)",
       R"( 3|    -       '   ' S I N D    |)",
       R"( 4|I     -       '   U N I     '|)",
       R"( 5|M       -         B O A      |)",
       R"( 6|P I       "       V       "  |)",
       R"( 7|R   '       '   ' I     '    |)",
       R"( 8|E E K '       U N R I D     =|)",
       R"( 9|S M I D G E O N ' A G E '    |)",
       R"(10|s "       "   F I L L E T S  |)",
       R"(11|E       F A V E L   U R E    |)",
       R"(12|D     Z A X   T I     E E   '|)",
       R"(13|  T W A Y   ' T A       T    |)",
       R"(14|L O O S   J O E   "     H U P|)",
       R"(15|O R T ' B O   R A N c H E R A|)"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("C").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 395, 459, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"6A ..C (score = 7)"});
}

TEST_F(AlphaBetaPlayerTest, Eik) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: false
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 32
        caps_per_ply: 16
        caps_per_ply: 8
        caps_per_ply: 4
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O      olaugh       LQ    465    )", 
       R"(   ------------------------------  -> cwac         EIK   524    )", 
       R"( 1|=     '     F E       V L O G| --Speedy Player's choices--   )",
       R"( 2|  C       P A m   A   I   N  | best  O6  EIK    25 100.0%    )",
       R"( 3|  L -     O P E ' U   N - N  |  6.00 13G EIK    19 100.0%    )",
       R"( 4|V A N G   R   R   d   A   I '|  8.00 6M  (AG)E  4  100.0% IK )",
       R"( 5|  R     A N T A C I D S   N  |  9.00 H10 (H)IKE 16 100.0%    )",
       R"( 6|  I O   B Y   U   L     A G  |  10.0 3B  (L)I   8  100.0% EK )",
       R"( 7|D E X       ' D E E     '    |  12.0 H10 (H)I   5  100.0% EK )",
       R"( 8|O S   Z O S T E R S   '     =|  12.0 K5  (D)I   5  100.0% EK )",
       R"( 9|J   T O R I I   R       '    |  13.0 11H EIK    12 100.0%    )",
       R"(10|O B I A   F E H   "       "  |  13.0 M6  (A)KE  12 100.0% I  )",
       R"(11|      E W T         -        |  14.0 B10 (B)I   4  100.0% EK )",
       R"(12|'     A E     '       -     '| --Tracking--------------------)",
       R"(13|    D E Y   '   '       -    | LQ  2                         )",
       R"(14|  - U     M M     "       -  |                               )",
       R"(15|W H I T E O U T       '     =|                               )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("EIK").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 465, 524, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"O6 EIK (score = 25)"});
}

TEST_F(AlphaBetaPlayerTest, Drent) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: true        
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 32
        caps_per_ply: 16
        caps_per_ply: 8
        caps_per_ply: 4
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O      cwac      BFIMQTY   388      )",
       R"(   ------------------------------  -> olaugh    GNDRTIN   381      )",
       R"( 1|O B O '       =       T     =| --Speedy Player's choices--      )",
       R"( 2|H O O     "       "   E   -  | best *2J DR(E)NT  20 100.0% GIN  )",
       R"( 3|O R T       '   '     L -    |  4.00 2J GR(E)IN  20 100.0% DTN  )",
       R"( 4|' A   -       '       E     '|  6.00 2K R(E)NIG  12 100.0% DTN  )",
       R"( 5|  C     -           - G      |  6.00 2K D(E)NT   10 100.0% GRIN )",
       R"( 6|  E       "       W R A W L S|  7.00 2K R(E)ND   10 100.0% GTIN )",
       R"( 7|U S '     Z A N J A     '   A|  7.00 2I RID(E)NT 18 100.0% GN   )",
       R"( 8|N     '       E U R I P I   V|  8.00 2K T(E)RN   8  100.0% GDIN )",
       R"( 9|D   '       V E G     A '    |  12.0 4K R(E)ND   5  100.0% GTIN )",
       R"(10|e "       "       "   L   "  |  16.0 4K T(E)ND   5  100.0% GRIN )",
       R"(11|I       -           K I R    |  16.0 4K D(E)NT   5  100.0% GRIN )",
       R"(12|F A R O u C H E     U N I   '| --Tracking-----------------------)",
       R"(13|I   -       '   '   T O N    | BFIMQTY  7                       )",
       R"(14|E X     E A L E   Y A D   -  |                                  )",
       R"(15|S U E T E   I M P O S E D   =|                                  )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("TRNDING").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 381, 388, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"2J DR.NT (score = 20)"});
}

TEST_F(AlphaBetaPlayerTest, Reverb) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::AlphaBetaPlayerConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        id: 1
        name: "AlphaBetaPlayer"
        nickname: "A"
        anagram_map_file: "src/scrabble/testdata/csw21.qam"
        board_layout_file: "src/scrabble/testdata/scrabble_board.textproto"
        tiles_file: "src/scrabble/testdata/english_scrabble_tiles.textproto"
        leaves_file: "src/scrabble/testdata/csw_scrabble_macondo.qlv"
        plies: 20
        detect_stuck_tiles: false
        stuck_tiles_left_multiplier: 0.2
        stuck_leave_score_multiplier: 1.0
        stuck_leave_value_multiplier: 0.2
        opp_stuck_score_multiplier: 2.0
        unstuck_leave_score_weight: 0.6
        unstuck_leave_value_weight: 0.08
        caps_per_ply: 32
        caps_per_ply: 16
        caps_per_ply: 8
        caps_per_ply: 4
        caps_per_ply: 2
        )",
                                                config);
  auto player = absl::make_unique<AlphaBetaPlayer>(*config);
  ASSERT_NE(player, nullptr);

  DataManager* dm = DataManager::GetInstance();
  const Tiles* tiles =
      dm->GetTiles("src/scrabble/testdata/english_scrabble_tiles.textproto");
  EXPECT_NE(tiles, nullptr);

  Board board;
  board.SetLetters(
      {R"(   A B C D E F G H I J K L M N O   -> cwac      EIBRSVW   350  )", 
       R"(   ------------------------------     olaugh    CFGIN     502  )",
       R"( 1|W     V I R E O   T A X I E D| --Speedy Player's choices--  )",
       R"( 2|Y - D A T E L I N E   U S E  | best  F1  (RE)VERB  30 ISW   )",
       R"( 3|C H O U     '   '   Q   -    |  1.00 M11 B(AR)R(A) 12 EISVW )",
       R"( 4|H     T       '     A G     '|  2.00 11M W(a)B     17 EIRSV )",
       R"( 5|      E R           N A      |  9.00 12B VIBS      26 ERW   )",
       R"( 6|  "     A "       J A M B "  |  9.00 12A BREWS     34 IV    )",
       R"( 7|    '   O   ' F O O T M E n  |  17.0 11D W(AI)R    7  EIBSV )",
       R"( 8|=     P U N T E R   S Y N O D|  18.0 F4  REW       30 IBSV  )",
       R"( 9|    P O L I O S '       E N  |  23.0 6D  S(A)B     20 EIRVW )",
       R"(10|  "     I D       "       T  |  28.0 11M W(a)R     13 EIBSV )",
       R"(11|        A I         -     a  |  46.1*F5  EW        29 IBRSV )",
       R"(12|'     -       '       G A L E| --Tracking-------------------)",
       R"(13|    -       '   '   L U R K  | CFGIN  5                     )",
       R"(14|  -       "       "       E  |                              )",
       R"(15|=     '       =       Z A R I|                              )"},
      *tiles);

  const BoardLayout* layout =
      dm->GetBoardLayout("src/scrabble/testdata/scrabble_board.textproto");
  std::stringstream ss1;
  layout->DisplayBoard(board, *tiles, ss1);
  LOG(INFO) << "board:" << std::endl << ss1.str();

  const Rack rack(tiles->ToLetterString("EIBRSVW").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout, board, 1, 2, rack, 350, 502, 0, absl::Minutes(25), 0, *tiles);
  std::stringstream ss2;
  pos->Display(ss2);
  LOG(INFO) << "position:" << std::endl << ss2.str();
  auto* computer_player = static_cast<ComputerPlayer*>(player.get());
  const auto move = computer_player->ChooseBestMove(nullptr, *pos);
  ExpectMove(move, {"F1 ..VERB (score = 30)"});
}