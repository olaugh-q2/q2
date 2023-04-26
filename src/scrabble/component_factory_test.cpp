#include <google/protobuf/descriptor.h>
#include <google/protobuf/text_format.h>
#include <gtest/gtest.h>

#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/unseen_tiles_predicate.h"

using ::google::protobuf::Arena;

class ComponentFactoryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    PassingPlayer::Register();
    UnseenTilesPredicate::Register();
  }
};

TEST_F(ComponentFactoryTest, CreateComputerPlayer) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::PassingPlayerConfig>(&arena);
  config->set_id(1);
  config->set_name("Passing Player");
  config->set_nickname("passing");
  std::unique_ptr<ComputerPlayer> player =
      ComponentFactory::GetInstance()->CreateComputerPlayer(*config);
  EXPECT_NE(player, nullptr);
  EXPECT_EQ(player->Id(), 1);
  EXPECT_EQ(player->Name(), "Passing Player");
  EXPECT_EQ(player->Nickname(), "passing");
  //EXPECT_TRUE(false);
}

TEST_F(ComponentFactoryTest, CreatePredicate) {
  Arena arena;
  auto config = Arena::CreateMessage<q2::proto::UnseenTilesPredicateConfig>(&arena);
  config->set_max_unseen_tiles(93);
  config->set_min_unseen_tiles(93);
  std::unique_ptr<Predicate> player =
      ComponentFactory::GetInstance()->CreatePredicate(*config);
  EXPECT_NE(player, nullptr);
}