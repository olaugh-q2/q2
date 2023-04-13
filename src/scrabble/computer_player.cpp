#include "src/scrabble/computer_player.h"

ComputerPlayer::~ComputerPlayer() {}

void ComputerPlayer::SetStartOfTurnTime() { start_of_turn_time_ = absl::Now(); }
