#include "src/scrabble/component_factory.h"

#include "glog/logging.h"
#include "src/scrabble/player.h"

void ComponentFactory::RegisterComputerPlayer(
    const google::protobuf::Descriptor* descriptor,
    std::function<
        std::unique_ptr<ComputerPlayer>(const google::protobuf::Message&)>
        creator) {
  computer_player_creators_.emplace(descriptor, creator);
}

std::unique_ptr<ComputerPlayer> ComponentFactory::CreateComputerPlayer(
    const google::protobuf::Message& message) const {
  const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
  auto it = computer_player_creators_.find(descriptor);
  if (it == computer_player_creators_.end()) {
    LOG(INFO) << "No ComputerPlayer creator registered for "
              << descriptor->full_name();
    return nullptr;
  }
  return it->second(message);
}