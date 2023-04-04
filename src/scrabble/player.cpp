#include "src/scrabble/player.h"

#include "absl/strings/str_cat.h"

void Player::Display(std::ostream& os) const {
  os << absl::StrCat("Player ", id_, ": ", name_);
  if (player_type_ == PlayerType::Human) {
    const std::string human(" (human)");
    os << human;
  } else {
    const std::string robot(" (robot)");
    os << robot;
  }
}