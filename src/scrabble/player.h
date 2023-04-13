#ifndef SRC_SCRABBLE_PLAYER_H
#define SRC_SCRABBLE_PLAYER_H

#include <string>

#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"

class Player {
 public:
  enum PlayerType { Human, Computer };

  Player(std::string& name, std::string& nickname, PlayerType player_type,
         int id)
      : name_(name), nickname_(nickname), player_type_(player_type), id_(id) {}
  virtual Move ChooseBestMove(const GamePosition& position) = 0;

  std::string Name() const { return name_; }
  std::string Nickname() const { return nickname_; }
  PlayerType GetPlayerType() const { return player_type_; }
  void Display(std::ostream& os) const;
  int Id() const { return id_; }

 private:
  std::string name_;
  std::string nickname_;
  const PlayerType player_type_;
  const int id_;
};

#endif  // SRC_SCRABBLE_PLAYER_H