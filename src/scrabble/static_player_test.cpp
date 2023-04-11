#include "src/scrabble/static_player.h"

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

std::unique_ptr<Tiles> tiles_;
std::unique_ptr<Leaves> leaves_;
std::unique_ptr<AnagramMap> anagram_map_;
std::unique_ptr<BoardLayout> layout_;

class StaticPlayerTest : public ::testing::Test {
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
  }

  void ExpectMove(const Move& move, const std::string& expected) {
    std::stringstream ss;
    move.Display(*tiles_, ss);
    const auto actual = ss.str();
    EXPECT_EQ(actual, expected);
  }
};

TEST_F(StaticPlayerTest, ChooseBestMove) {
  StaticPlayer player(1, *anagram_map_, *layout_, *tiles_, *leaves_);
  const Board board;
  const Rack rack(tiles_->ToLetterString("OLAUGHS").value());
  auto pos = absl::make_unique<GamePosition>(
      *layout_, board, 1, 2, rack, 0, 0, 0, absl::Minutes(25), 0, *tiles_);
  // This is just figuring out for myself how the casting works with the virtual
  // function ChooseBestMove which exists in ComputerPlayer (but not Player) and is
  // overridden by StaticPlayer.      
  Player* player_ptr = &player;
  auto* computer_player = static_cast<ComputerPlayer*>(player_ptr);
  //ComputerPlayer* computer_player = static_cast<ComputerPlayer*>(player_ptr);
  const auto move = computer_player->ChooseBestMove(*pos);
  ExpectMove(move, "8F GOULASH (score = 80)");
}