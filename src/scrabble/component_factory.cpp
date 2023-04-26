#include "src/scrabble/component_factory.h"

#include "glog/logging.h"
#include "src/scrabble/player.h"

void ComponentFactory::RegisterComputerPlayer(
    const google::protobuf::Descriptor* descriptor,
    std::function<
        std::unique_ptr<ComputerPlayer>(const google::protobuf::Message&)>
        creator) {
  LOG(INFO) << "Registering ComputerPlayer creator for "
            << descriptor->full_name();
  computer_player_creators_.emplace(descriptor, creator);
}

void ComponentFactory::RegisterPredicate(
    const google::protobuf::Descriptor* descriptor,
    std::function<std::unique_ptr<Predicate>(const google::protobuf::Message&)>
        creator) {
  LOG(INFO) << "Registering Predicate creator for " << descriptor->full_name();
  predicate_creators_.emplace(descriptor, creator);
}

std::unique_ptr<ComputerPlayer> ComponentFactory::CreateComputerPlayer(
    const google::protobuf::Message& message) const {
  // LOG(INFO) << "message: " << message.DebugString();
  const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
  auto it = computer_player_creators_.find(descriptor);
  if (it == computer_player_creators_.end()) {
    LOG(FATAL) << "No ComputerPlayer creator registered for "
               << descriptor->full_name();
    return nullptr;
  }
  return it->second(message);
}

std::unique_ptr<Predicate> ComponentFactory::CreatePredicate(
    const google::protobuf::Message& message) const {
  // LOG(INFO) << "message: " << message.DebugString();
  const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
  auto it = predicate_creators_.find(descriptor);
  if (it == predicate_creators_.end()) {
    LOG(FATAL) << "No Predicate creator registered for "
               << descriptor->full_name();
    return nullptr;
  }
  return it->second(message);
}