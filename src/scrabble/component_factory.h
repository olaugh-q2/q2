#ifndef SRC_SCRABBLE_COMPONENT_FACTORY_H
#define SRC_SCRABBLE_COMPONENT_FACTORY_H

#include <google/protobuf/message.h>

#include <functional>
#include <memory>
#include <unordered_map>

#include "src/scrabble/computer_player.h"
#include "src/scrabble/predicate.h"

class ComponentFactory {
 public:
  static ComponentFactory* GetInstance() {
    static ComponentFactory instance;
    return &instance;
  }
  std::unique_ptr<ComputerPlayer> CreateComputerPlayer(
      const google::protobuf::Message& message) const;

  void RegisterComputerPlayer(const google::protobuf::Descriptor* descriptor,
                              std::function<std::unique_ptr<ComputerPlayer>(
                                  const google::protobuf::Message&)>
                                  creator);

  std::unique_ptr<Predicate> CreatePredicate(
      const google::protobuf::Message& message) const;

  void RegisterPredicate(const google::protobuf::Descriptor* descriptor,
                         std::function<std::unique_ptr<Predicate>(
                             const google::protobuf::Message&)>
                             creator);

  static std::unique_ptr<ComputerPlayer> CreatePlayerFromConfig(
      const q2::proto::ComputerPlayerConfig& config);

  static std::unique_ptr<Predicate> CreatePredicateFromConfig(
      const q2::proto::PredicateConfig& config);

 private:
  ComponentFactory() = default;
  std::unordered_map<const google::protobuf::Descriptor*,
                     std::function<std::unique_ptr<ComputerPlayer>(
                         const google::protobuf::Message&)>>
      computer_player_creators_;

  std::unordered_map<const google::protobuf::Descriptor*,
                     std::function<std::unique_ptr<Predicate>(
                         const google::protobuf::Message&)>>
      predicate_creators_;
};

#endif  // SRC_SCRABBLE_COMPONENT_FACTORY_H