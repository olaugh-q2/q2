#include "src/scrabble/game.h"

#include "absl/memory/memory.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/move_finder.h"
#include "src/scrabble/player.h"
#include "src/scrabble/static_player.h"
#include "src/scrabble/tiles.h"

using ::testing::ElementsAre;

std::unique_ptr<Tiles> tiles_;
std::unique_ptr<Leaves> leaves_;
std::unique_ptr<AnagramMap> anagram_map_;
std::unique_ptr<BoardLayout> layout_;
std::unique_ptr<MoveFinder> move_finder_;

class GameTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    LOG(INFO) << "tiles ok";
    anagram_map_ = AnagramMap::CreateFromBinaryFile(
        *tiles_, "src/scrabble/testdata/csw21.qam");
    LOG(INFO) << "anagram map ok";
    layout_ = absl::make_unique<BoardLayout>(
        "src/scrabble/testdata/scrabble_board.textproto");
    LOG(INFO) << "board layout ok";
    leaves_ = Leaves::CreateFromBinaryFile(
        *tiles_, "src/scrabble/testdata/csw_scrabble_macondo.qlv");
    move_finder_ = absl::make_unique<MoveFinder>(*anagram_map_, *layout_,
                                                 *tiles_, *leaves_);
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }
};

class TestHumanPlayer : public Player {
 public:
  TestHumanPlayer(std::string name, std::string nickname, int id)
      : Player(name, nickname, Player::Human, id) {}

  Move ChooseBestMove(const GamePosition& position) override;
};

Move TestHumanPlayer::ChooseBestMove(const GamePosition& position) {
  CHECK(false) << "TestHumanPlayer::ChooseBestMove(..) not implemented, should "
                  "not be called";
  Move move;
  return move;
}

class PassingPlayer : public ComputerPlayer {
 public:
  PassingPlayer(int id) : ComputerPlayer("Passing Player", "passer", id) {}

  Move ChooseBestMove(const GamePosition& position) override;
};

Move PassingPlayer::ChooseBestMove(const GamePosition& position) {
  LOG(INFO) << "PassingPlayer::ChooseBestMove(..)";
  SetStartOfTurnTime();
  Move move;
  move.SetScore(0);
  return move;
}

TEST_F(GameTest, ConstructAndDisplay) {
  TestHumanPlayer a("Alice", "A", 1);
  TestHumanPlayer b("Bob", "B", 2);
  const std::vector<Player*> players = {&a, &b};
  Game game(*layout_, players, *tiles_, absl::Minutes(25));
  std::stringstream ss;
  game.Display(ss);
  LOG(INFO) << ss.str();
  EXPECT_EQ(ss.str(), R"(Player 1: Alice (human)
Player 2: Bob (human)

No positions.
)");
}

TEST_F(GameTest, CreateInitialPosition) {
  TestHumanPlayer a("Alice", "A", 1);
  TestHumanPlayer b("Bob", "B", 2);
  const std::vector<Player*> players = {&a, &b};
  Game game1(*layout_, players, *tiles_, absl::Minutes(25));
  const Bag bag(*tiles_);
  const std::vector<uint64_t> dividends = {1, 10, 150};
  game1.CreateInitialPosition(bag, dividends);
  std::stringstream ss1;
  game1.Display(ss1);
  LOG(INFO) << ss1.str();
  EXPECT_EQ(ss1.str(), R"(Player 1: Alice (human)
Player 2: Bob (human)

Current position: 
  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　　　－　　　＇　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
Player 1 holds ??ZYYXW on 0 to opp's 0 [25:00]
Unseen: AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVW
)");

  Game game2(*layout_, players, *tiles_, absl::Minutes(25));
  // Nondeterministic random bag and dividends but the string should be the same
  // length as in ss1.
  game2.CreateInitialPosition();
  std::stringstream ss2;
  game2.Display(ss2);
  LOG(INFO) << ss2.str();
  EXPECT_EQ(ss2.str().size(), ss1.str().size());
}

TEST_F(GameTest, AddNextPosition) {
  // Setting these to 3 and 4 just to make it clear that we're using ids and not
  // indices.
  TestHumanPlayer a("Alice", "A", 3);
  TestHumanPlayer b("Bob", "B", 4);
  std::vector<Player*> players = {&a, &b};
  Game game(*layout_, players, *tiles_, absl::Minutes(25));
  Bag bag(*tiles_);
  bag.SetLetters({L('I'), L('I'), L('I'), L('I'), L('I'), L('I'), L('I'),
                  L('A'), L('A'), L('A'), L('A'), L('A'), L('A'), L('A'),
                  L('E'), L('E'), L('E'), L('E'), L('E'), L('E'), L('E'),
                  L('S'), L('H'), L('G'), L('U'), L('A'), L('L'), L('O')});
  const std::vector<uint64_t> dividends = {0};
  game.CreateInitialPosition(bag, dividends);
  std::stringstream ss1;
  game.Display(ss1);
  LOG(INFO) << ss1.str();
  EXPECT_EQ(ss1.str(), R"(Player 3: Alice (human)
Player 4: Bob (human)

Current position: 
  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　　　－　　　＇　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
Player 3 holds OLAUGHS on 0 to opp's 0 [25:00]
Unseen: AAAAAAAEEEEEEEIIIIIII
)");

  auto goulash = Move::Parse("8F GOULASH", *tiles_);
  goulash->SetScore(80);
  LetterString leave1;  // empty;
  goulash->SetLeave(leave1);
  game.AddNextPosition(*goulash, absl::Minutes(1));
  std::stringstream ss2;
  game.Display(ss2);
  LOG(INFO) << ss2.str();
  EXPECT_EQ(ss2.str(), R"(Player 3: Alice (human)
Player 4: Bob (human)

Game log: 
  A played: 8F GOULASH (score = 80)

Current position: 
  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　ＧＯＵＬＡＳＨ　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
Player 4 holds EEEEEEE on 0 to opp's 80 [25:00]
Unseen: AAAAAAAIIIIIII
)");

  auto exchange = Move::Parse("EXCH EEEEEE", *tiles_);
  exchange->SetScore(0);
  LetterString leave2 = LS("E");
  exchange->SetLeave(leave2);
  game.AddNextPosition(*exchange, absl::Seconds(10));
  std::stringstream ss3;
  game.Display(ss3);
  LOG(INFO) << ss3.str();
  // EXPECT_TRUE(false);
  EXPECT_EQ(ss3.str(), R"(Player 3: Alice (human)
Player 4: Bob (human)

Game log: 
  A played: 8F GOULASH (score = 80)
  B played: EXCH EEEEEE (score = 0)

Current position: 
  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ
 1＝　　＇　　　＝　　　＇　　＝
 2　－　　　＂　　　＂　　　－　
 3　　－　　　＇　＇　　　－　　
 4＇　　－　　　＇　　　－　　＇
 5　　　　－　　　　　－　　　　
 6　＂　　　＂　　　＂　　　＂　
 7　　＇　　　＇　＇　　　＇　　
 8＝　　＇　ＧＯＵＬＡＳＨ　　＝
 9　　＇　　　＇　＇　　　＇　　
10　＂　　　＂　　　＂　　　＂　
11　　　　－　　　　　－　　　　
12＇　　－　　　＇　　　－　　＇
13　　－　　　＇　＇　　　－　　
14　－　　　＂　　　＂　　　－　
15＝　　＇　　　＝　　　＇　　＝
[Consecutive scoreless turns preceding this position: 1]
Player 3 holds AAAAAAA on 80 to opp's 0 [24:00]
Unseen: EEEEEEEIIIIIII
)");
}

TEST_F(GameTest, SixPassGame) {
  PassingPlayer a(1);
  PassingPlayer b(2);
  std::vector<Player*> players = {&a, &b};
  Game game(*layout_, players, *tiles_, absl::Minutes(25));
  game.CreateInitialPosition();
  game.FinishWithComputerPlayers();
  EXPECT_THAT(game.Scores(), ElementsAre(0, 0));
  EXPECT_EQ(game.Positions().size(), 7);
  EXPECT_EQ(game.Positions()[6].ScorelessTurns(), 6);
}

TEST_F(GameTest, StaticPlayerPlays) {
  StaticPlayer a(1, *anagram_map_, *layout_, *tiles_, *leaves_);
  StaticPlayer b(2, *anagram_map_, *layout_, *tiles_, *leaves_);
  std::vector<Player*> players = {&a, &b};
  for (int i = 0; i < 10; i++) {
    Game game(*layout_, players, *tiles_, absl::Minutes(25));
    game.CreateInitialPosition();
    game.FinishWithComputerPlayers();
    std::stringstream ss;
    game.Display(ss);
    LOG(INFO) << ss.str();
    EXPECT_GE(game.Positions().size(), 7);
  }
}