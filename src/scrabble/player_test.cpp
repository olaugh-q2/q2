#include "src/scrabble/player.h"

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class PlayerTest : public ::testing::Test {
 protected:
  PlayerTest() {}
  ~PlayerTest() override {}
  void SetUp() override {}
  void TearDown() override {}
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

TEST_F(PlayerTest, Display) {
  TestHumanPlayer player("John O'Laughlin", "olaugh", 1);
  std::stringstream ss;
  player.Display(ss);
  EXPECT_EQ(ss.str(), "Player 1: John O'Laughlin (human)");
  EXPECT_EQ(player.Nickname(), "olaugh");
}