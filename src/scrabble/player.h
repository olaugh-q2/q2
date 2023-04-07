#ifndef SRC_SCRABBLE_PLAYER_H
#define SRC_SCRABBLE_PLAYER_H

#include <string>

class Player {
 public:
  enum PlayerType { Human, Computer };

  Player(const std::string& name, const std::string& nickname,
         PlayerType player_type, int id)
      : name_(name), nickname_(nickname), player_type_(player_type), id_(id) {}

  std::string Name() const { return name_; }
  std::string Nickname() const { return nickname_; }
  PlayerType GetPlayerType() const { return player_type_; }
  void Display(std::ostream& os) const;

 private:
  std::string name_;
  std::string nickname_;
  const PlayerType player_type_;
  const int id_;
};

#endif  // SRC_SCRABBLE_PLAYER_H