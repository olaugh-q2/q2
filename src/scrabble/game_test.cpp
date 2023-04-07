#include "src/scrabble/game.h"

#include "absl/memory/memory.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/move_finder.h"
#include "src/scrabble/player.h"
#include "src/scrabble/tiles.h"

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
};

TEST_F(GameTest, ConstructAndDisplay) {
  const Player a("Alice", "A", Player::PlayerType::Human, 1);
  const Player b("Bob", "B", Player::PlayerType::Human, 2);
  const std::vector<Player> players = {a, b};
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
  const Player a("Alice", "A", Player::PlayerType::Human, 1);
  const Player b("Bob", "B", Player::PlayerType::Human, 2);
  const std::vector<Player> players = {a, b};
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
Player 0 holds ??ZYYXW on 0 to opp's 0 [25:00]
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
