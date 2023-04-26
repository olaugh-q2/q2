#include "src/scrabble/unseen_tiles_predicate.h"

#include <google/protobuf/text_format.h>

using ::google::protobuf::Arena;

#include "game.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/player.h"
#include "src/scrabble/tiles.h"

std::unique_ptr<Tiles> tiles_;
std::unique_ptr<BoardLayout> layout_;

class UnseenTilesPredicateTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
    LOG(INFO) << "tiles ok";
    layout_ = absl::make_unique<BoardLayout>(
        "src/scrabble/testdata/scrabble_board.textproto");
    LOG(INFO) << "board layout ok";
  }
};
TEST_F(UnseenTilesPredicateTest, Evaluate100) {
  Arena arena;

  PassingPlayer a(1);
  PassingPlayer b(2);
  std::vector<Player*> players = {&a, &b};
  Game game(*layout_, players, *tiles_, absl::Minutes(25));
  game.CreateInitialPosition();

  auto spec1 =
      Arena::CreateMessage<q2::proto::UnseenTilesPredicateConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        min_unseen_tiles: 100
        max_unseen_tiles: 100
    )",
                                                spec1);
  UnseenTilesPredicate predicate1(*spec1);
  EXPECT_FALSE(predicate1.Evaluate(game.Positions()[0]));

  auto spec2 =
      Arena::CreateMessage<q2::proto::UnseenTilesPredicateConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        min_unseen_tiles: 93
        max_unseen_tiles: 100
    )",
                                                spec2);
  UnseenTilesPredicate predicate2(*spec2);
  EXPECT_TRUE(predicate2.Evaluate(game.Positions()[0]));

  auto spec3 =
      Arena::CreateMessage<q2::proto::UnseenTilesPredicateConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        min_unseen_tiles: 93
        max_unseen_tiles: 93
    )",
                                                spec3);
  UnseenTilesPredicate predicate3(*spec3);
  EXPECT_TRUE(predicate3.Evaluate(game.Positions()[0]));

  auto spec4 =
      Arena::CreateMessage<q2::proto::UnseenTilesPredicateConfig>(&arena);
  google::protobuf::TextFormat::ParseFromString(R"(
        min_unseen_tiles: 0
        max_unseen_tiles: 92
    )",
                                                spec3);
  UnseenTilesPredicate predicate4(*spec4);
  EXPECT_FALSE(predicate4.Evaluate(game.Positions()[0]));
}