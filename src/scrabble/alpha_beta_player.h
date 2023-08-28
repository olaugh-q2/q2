#ifndef SRC_SCRABBLE_ALPHA_BETA_PLAYER_H
#define SRC_SCRABBLE_ALPHA_BETA_PLAYER_H

#include "absl/memory/memory.h"
#include "src/anagram/anagram_map.h"
#include "src/leaves/leaves.h"
#include "src/scrabble/board_layout.h"
#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/data_manager.h"
#include "src/scrabble/game_position.h"
#include "src/scrabble/move.h"
#include "src/scrabble/move_finder.h"
#include "src/scrabble/rack.h"

class AlphaBetaPlayer : public ComputerPlayer {
 public:
  struct PlacedLetter {
    uint8_t row : 4;
    uint8_t col : 4;
    Letter letter;
  };
  class GameState {
   public:
    GameState(uint64_t on_move_rack_product, uint64_t off_move_rack_product,
              uint8_t num_placed_letters,
              std::array<PlacedLetter, 14> placed_letters)
        : on_move_rack_product_(on_move_rack_product),
          off_move_rack_product_(off_move_rack_product),
          num_placed_letters_(num_placed_letters),
          placed_letters_(placed_letters) {}

   private:
    uint64_t on_move_rack_product_;
    uint64_t off_move_rack_product_;
    uint8_t num_placed_letters_;
    std::array<PlacedLetter, 14> placed_letters_;
  };

  class GameNode {
   public:
    GameNode(const Move& move, GameNode* parent, float value, uint8_t depth)
        : move_(move), parent_(parent), value_(value), depth_(depth) {}
    const Move& GetMove() const { return move_; }
    const GameNode* Parent() const { return parent_; }
    float Value() const { return value_; }
    uint8_t Depth() const { return depth_; }
    void SetValue(float value) { value_ = value; }
    float CalculateValue(const GamePosition& position, bool on_turn,
                         bool terminal) {
      const float sign = on_turn ? 1.0 : -1.0;
      const float net = sign * (terminal ? move_.Equity() : move_.Score());
      // LOG(INFO) << "net: " << net;
      if (parent_ == nullptr) {
        return net;
      }
      return net + parent_->CalculateValue(position, !on_turn, false);
    }

   private:
    Move move_;
    GameNode* parent_;
    float value_;
    uint8_t depth_;
  };

  static void Register() {
    LOG(INFO) << "Registering AlphaBetaPlayer";
    ComponentFactory::GetInstance()->RegisterComputerPlayer(
        q2::proto::AlphaBetaPlayerConfig::descriptor(),
        [](const google::protobuf::Message& message) {
          return absl::make_unique<AlphaBetaPlayer>(
              dynamic_cast<const q2::proto::AlphaBetaPlayerConfig&>(message));
        });
  }

  explicit AlphaBetaPlayer(const q2::proto::AlphaBetaPlayerConfig& config)
      : ComputerPlayer(config.name(), config.nickname(), config.id()),
        tiles_(*DataManager::GetInstance()->GetTiles(config.tiles_file())),
        leaves_(*(DataManager::GetInstance()->GetLeaves(config.leaves_file()))),
        num_plies_(config.plies()),
        detect_stuck_tiles_(config.detect_stuck_tiles()),
        stuck_tiles_left_multiplier_(config.stuck_tiles_left_multiplier()),
        stuck_leave_score_multiplier_(config.stuck_leave_score_multiplier()),
        stuck_leave_value_multiplier_(config.stuck_leave_value_multiplier()),
        opp_stuck_score_multiplier_(config.opp_stuck_score_multiplier()),
        unstuck_leave_score_weight_(config.unstuck_leave_score_weight()),
        unstuck_leave_value_weight_(config.unstuck_leave_value_weight()) {
    move_finder_ = absl::make_unique<MoveFinder>(
        *DataManager::GetInstance()->GetAnagramMap(config.anagram_map_file()),
        *DataManager::GetInstance()->GetBoardLayout(config.board_layout_file()),
        tiles_, *DataManager::GetInstance()->GetLeaves(config.leaves_file()));
    for (const auto& cap_per_ply : config.caps_per_ply()) {
      caps_per_ply_.push_back(cap_per_ply);
    }
  }

  Move ChooseBestMove(const std::vector<GamePosition>* previous_position,
                      const GamePosition& position) override;

 private:
  GameNode* AlphaBeta(GameNode* parent, const GameState& state, bool on_turn,
                      int depth, float alpha, float beta, GamePosition* p);
  std::vector<Move> MovesAfterParent(const GameNode* parent,
                                     GamePosition* position);

  int CapAtPly(int ply) const {
    if (caps_per_ply_.empty()) {
      return 80000;
    } else if (ply >= caps_per_ply_.size()) {
      return caps_per_ply_.back();
    } else {
      return caps_per_ply_[ply];
    }
  }
  const Tiles& tiles_;
  const Leaves& leaves_;
  std::unique_ptr<MoveFinder> move_finder_;
  int num_plies_;
  bool detect_stuck_tiles_;
  float stuck_tiles_left_multiplier_;
  float stuck_leave_score_multiplier_;
  float stuck_leave_value_multiplier_;
  float opp_stuck_score_multiplier_;
  float unstuck_leave_score_weight_;
  float unstuck_leave_value_weight_;
  std::vector<std::unique_ptr<GameNode>> nodes_;
  GameNode* root_node_;
  std::vector<int> caps_per_ply_;
};

#endif  // SRC_SCRABBLE_ALPHA_BETA_PLAYER_H