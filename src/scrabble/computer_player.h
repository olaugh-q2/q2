#ifndef SRC_SCRABBLE_COMPUTER_PLAYER_H
#define SRC_SCRABBLE_COMPUTER_PLAYER_H

#include "absl/time/time.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"
#include "src/scrabble/player.h"

class ComputerPlayer : public Player {
 public:
  ComputerPlayer(const std::string& name, const std::string& nickname,
                 int id)
      : Player(name, nickname, Player::Computer, id) {}
  ~ComputerPlayer() {}

 virtual Move ChooseBestMove(const GamePosition& position) = 0;

 void SetStartOfTurnTime();

 private:
  absl::Time start_of_turn_time_;
};

#endif  // SRC_SCRABBLE_COMPUTER_PLAYER_H