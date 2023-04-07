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
  void SetUp() override {
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
