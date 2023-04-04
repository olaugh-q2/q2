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

TEST_F(PlayerTest, Display) {
  Player player("John O'Laughlin", "olaugh", Player::PlayerType::Human, 1);
  std::stringstream ss;
  player.Display(ss);
  EXPECT_EQ(ss.str(), "Player 1: John O'Laughlin (human)");
  EXPECT_EQ(player.Nickname(), "olaugh");
}