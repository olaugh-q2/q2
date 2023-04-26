#ifndef SRC_SCRABBLE_UNSEEN_TILES_PREDICATE_H
#define SRC_SCRABBLE_UNSEEN_TILES_PREDICATE_H

#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/predicate.h"

class UnseenTilesPredicate : public Predicate {
 public:
  static void Register() {
    ComponentFactory::GetInstance()->RegisterPredicate(
        q2::proto::UnseenTilesPredicateConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<UnseenTilesPredicate>(
              dynamic_cast<const q2::proto::UnseenTilesPredicateConfig&>(
                  message));
        });
  }

  explicit UnseenTilesPredicate(
      const q2::proto::UnseenTilesPredicateConfig& config)
      : Predicate(),
        min_unseen_tiles_(config.min_unseen_tiles()),
        max_unseen_tiles_(config.max_unseen_tiles()) {}

  bool Evaluate(const GamePosition& position) const override;

 private:
  const uint32_t min_unseen_tiles_;
  const uint32_t max_unseen_tiles_;
};

#endif  // SRC_SCRABBLE_UNSEEN_TILES_PREDICATE_H