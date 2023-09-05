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

void ComponentFactory::RegisterTileOrderingProvider(
    const google::protobuf::Descriptor* descriptor,
    std::function<
        std::unique_ptr<TileOrderingProvider>(const google::protobuf::Message&)>
        creator) {
  LOG(INFO) << "Registering TileOrderingProvider creator for "
            << descriptor->full_name();
  tile_ordering_provider_creators_.emplace(descriptor, creator);
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

std::unique_ptr<ComputerPlayer> ComponentFactory::CreatePlayerFromConfig(
    const q2::proto::ComputerPlayerConfig& config) {
  switch (config.player_case()) {
    case q2::proto::ComputerPlayerConfig::kPassingPlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.passing_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::kStaticPlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.static_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::kSpecializingPlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.specializing_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::kEndgamePlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.endgame_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::kAlphaBetaPlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.alpha_beta_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::kSimmingPlayerConfig:
      return ComponentFactory::GetInstance()->CreateComputerPlayer(
          config.simming_player_config());
      break;
    case q2::proto::ComputerPlayerConfig::PLAYER_NOT_SET:
      LOG(ERROR) << "No player type specified for player "
                 << config.DebugString();
      return nullptr;
  }
}

std::unique_ptr<Predicate> ComponentFactory::CreatePredicateFromConfig(
    const q2::proto::PredicateConfig& config) {
  switch (config.predicate_case()) {
    case q2::proto::PredicateConfig::kUnseenTilesPredicateConfig:
      return ComponentFactory::GetInstance()->CreatePredicate(
          config.unseen_tiles_predicate_config());
      break;
    case q2::proto::PredicateConfig::PREDICATE_NOT_SET:
      LOG(ERROR) << "No predicate type specified for predicate "
                 << config.DebugString();
      return nullptr;
  }
}

std::unique_ptr<TileOrderingProvider>
ComponentFactory::CreateTileOrderingProvider(
    const google::protobuf::Message& message) const {
  // LOG(INFO) << "message: " << message.DebugString();
  const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
  auto it = tile_ordering_provider_creators_.find(descriptor);
  if (it == tile_ordering_provider_creators_.end()) {
    LOG(FATAL) << "No TileOrderingProvider creator registered for "
               << descriptor->full_name();
    return nullptr;
  }
  return it->second(message);
}

std::unique_ptr<TileOrderingProvider>
ComponentFactory::CreateTileOrderingProviderFromConfig(
    const q2::proto::TileOrderingProviderConfig& config) {
  switch (config.provider_case()) {
    case q2::proto::TileOrderingProviderConfig::kTileOrderingCacheConfig:
      return ComponentFactory::GetInstance()->CreateTileOrderingProvider(
          config.tile_ordering_cache_config());
      break;
    case q2::proto::TileOrderingProviderConfig::PROVIDER_NOT_SET:
      LOG(ERROR) << "No tile ordering provider type specified for tile "
                    "ordering provider "
                 << config.DebugString();
      return nullptr;
  }
}

std::unique_ptr<TileOrderingProvider>
    ComponentFactory::tile_ordering_provider_ = nullptr;

void ComponentFactory::CreateSingletonComponents(
    const q2::proto::SingletonComponents& singleton_components) {
  LOG(INFO) << "Creating singleton components";
  tile_ordering_provider_ = CreateTileOrderingProviderFromConfig(
      singleton_components.tile_ordering_provider_config());
}