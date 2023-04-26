#include "src/scrabble/tournament_runner.h"

#include <thread>

#include "src/scrabble/component_factory.h"
#include "src/scrabble/computer_player.h"
#include "src/scrabble/computer_players.pb.h"
#include "src/scrabble/data_manager.h"
#include "src/scrabble/game.h"
#include "src/scrabble/passing_player.h"
#include "src/scrabble/static_player.h"

using ::google::protobuf::Arena;

void TournamentRunner::RunGames(
    const Tiles& tiles, const BoardLayout& board_layout, int thread_index,
    int start_index, int num_pairs,
    std::vector<std::unique_ptr<q2::proto::GameResult>>* results) const {
  absl::BitGen gen;
  std::vector<Player*> players;
  for (int i = 0; i < 2; i++) {
    auto& player = players_[thread_index * 2 + i];
    players.push_back(player.get());
  }
  for (int i = 0; i < num_pairs; i++) {
    Bag bag(tiles);
    bag.Shuffle(gen);
    std::vector<uint64_t> exchange_insertion_dividends;
    for (int i = 0; i < 1000; ++i) {
      exchange_insertion_dividends.push_back(absl::Uniform<uint64_t>(gen));
    }
    for (int j = 0; j < 2; j++) {
      if (j == 1) {
        std::reverse(players.begin(), players.end());
      }
      int game_number = start_index + i * 2 + j;
      // LOG(INFO) << "Game " << game_number;
      Game game(board_layout, players, tiles, absl::Minutes(25));
      game.CreateInitialPosition(bag, exchange_insertion_dividends);
      game.FinishWithComputerPlayers();
      (*results)[game_number] = absl::make_unique<q2::proto::GameResult>();
      game.WriteProto((*results)[game_number].get());
    }
  }
}

namespace {
void DoGameStats(
    const q2::proto::GameResult* game,
    absl::flat_hash_map<int, std::unique_ptr<q2::proto::PlayerResults>>*
        player_results) {
  int p0 = game->player_ids(0);
  int p1 = game->player_ids(1);
  auto* r0 = (*player_results)[p0].get();
  auto* r1 = (*player_results)[p1].get();
  r0->set_total_score(r0->total_score() + game->player_scores(0));
  r0->set_total_opponent_score(r0->total_opponent_score() +
                               game->player_scores(1));
  r0->set_score_difference_sum_of_squares(
      r0->score_difference_sum_of_squares() +
      (game->player_scores(0) - game->player_scores(1)) *
          (game->player_scores(0) - game->player_scores(1)));
  r1->set_total_score(r1->total_score() + game->player_scores(1));
  r1->set_total_opponent_score(r1->total_opponent_score() +
                               game->player_scores(0));
  r1->set_score_difference_sum_of_squares(
      r1->score_difference_sum_of_squares() +
      (game->player_scores(1) - game->player_scores(0)) *
          (game->player_scores(1) - game->player_scores(0)));

  if (game->player_scores(0) > game->player_scores(1)) {
    r0->set_num_wins(r0->num_wins() + 1);
    r1->set_num_losses(r1->num_losses() + 1);
  } else if (game->player_scores(0) < game->player_scores(1)) {
    r0->set_num_losses(r0->num_losses() + 1);
    r1->set_num_wins(r1->num_wins() + 1);
  } else {
    r0->set_num_draws(r0->num_draws() + 1);
    r1->set_num_draws(r1->num_draws() + 1);
  }
}
void DoPairStats(
    const q2::proto::GameResult* g0, const q2::proto::GameResult* g1,
    absl::flat_hash_map<int, std::unique_ptr<q2::proto::PlayerResults>>*
        player_results) {
  int p0 = g0->player_ids(0);
  int p1 = g0->player_ids(1);
  auto* r0 = (*player_results)[p0].get();
  auto* r1 = (*player_results)[p1].get();
  int p0halves = 0;
  if (g0->player_scores(0) > g0->player_scores(1)) {
    p0halves += 2;
  }
  if (g1->player_scores(1) > g0->player_scores(0)) {
    p0halves += 2;
  }
  if (g0->player_scores(0) == g0->player_scores(1)) {
    p0halves += 1;
  }
  if (g1->player_scores(1) == g1->player_scores(0)) {
    p0halves += 1;
  }
  if (p0halves == 4) {
    r0->set_mirrored_sweeps(r0->mirrored_sweeps() + 1);
    r1->set_mirrored_sombreros(r1->mirrored_sombreros() + 1);
  } else if (p0halves == 3) {
    r0->set_mirrored_3quart(r0->mirrored_3quart() + 1);
    r1->set_mirrored_1quart(r1->mirrored_1quart() + 1);
  } else if (p0halves == 2) {
    r0->set_mirrored_splits(r0->mirrored_splits() + 1);
    r1->set_mirrored_splits(r1->mirrored_splits() + 1);
  } else if (p0halves == 1) {
    r0->set_mirrored_1quart(r0->mirrored_1quart() + 1);
    r1->set_mirrored_3quart(r1->mirrored_3quart() + 1);
  } else if (p0halves == 0) {
    r0->set_mirrored_sombreros(r0->mirrored_sombreros() + 1);
    r1->set_mirrored_sweeps(r1->mirrored_sweeps() + 1);
  }
}
}  // namespace
void TournamentRunner::Run() {
  LOG(INFO) << "Run()";

  const auto& data = spec_.data_collection();
  if (data.board_files_size() != 1) {
    LOG(FATAL) << "Only one board file supported";
  }
  if (data.tiles_files_size() != 1) {
    LOG(FATAL) << "Only one tiles file supported";
  }
  if (spec_.players_size() < 2) {
    LOG(FATAL) << "At least two players required";
  }
  if (spec_.number_of_rounds() < 1) {
    LOG(FATAL) << "At least one round required";
  }
  if (spec_.number_of_rounds() < 1) {
    LOG(FATAL) << "At least one thread required";
  }
  DataManager::GetInstance()->LoadData(data);
  for (int i = 0; i < spec_.number_of_threads(); ++i) {
    for (const auto& player_spec : spec_.players()) {
      switch (player_spec.player_case()) {
        case q2::proto::ComputerPlayerConfig::kPassingPlayerConfig:
          players_.push_back(
              ComponentFactory::GetInstance()->CreateComputerPlayer(
                  player_spec.passing_player_config()));
          break;
        case q2::proto::ComputerPlayerConfig::kStaticPlayerConfig:
          players_.push_back(
              ComponentFactory::GetInstance()->CreateComputerPlayer(
                  player_spec.static_player_config()));
          break;
        case q2::proto::ComputerPlayerConfig::PLAYER_NOT_SET:
          LOG(ERROR) << "No player type specified for player "
                     << player_spec.DebugString();
      }
    }
  }
  const auto& board_layout =
      *DataManager::GetInstance()->GetBoardLayout(data.board_files(0));
  const auto& tiles =
      *DataManager::GetInstance()->GetTiles(data.tiles_files(0));
  const int number_of_pairs = spec_.number_of_rounds() / 2;
  std::vector<std::unique_ptr<q2::proto::GameResult>> results;
  results.resize(spec_.number_of_rounds());
  std::vector<std::thread> threads;
  const int pairs_per_thread = number_of_pairs / spec_.number_of_threads();
  const int remainder = number_of_pairs % spec_.number_of_threads();
  for (int i = 0; i < spec_.number_of_threads(); ++i) {
    int start_index = i * pairs_per_thread;
    int pairs_to_run = pairs_per_thread;
    if (i < remainder) {
      start_index += i;
      pairs_to_run++;
    } else {
      start_index += remainder;
    }
    threads.emplace_back([this, &tiles, &board_layout, i, start_index,
                          pairs_to_run, &results]() {
      RunGames(tiles, board_layout, i, start_index * 2, pairs_to_run, &results);
    });
  }
  // Wait for all threads to finish
  for (auto& thread : threads) {
    thread.join();
  }
  // for (const auto& result : results) {
  //   std::cout << result->DebugString() << std::endl;
  // }
  LOG(INFO) << "Got " << results.size() << " results";
  Arena arena;
  auto tournament_results =
      Arena::CreateMessage<q2::proto::TournamentResults>(&arena);
  absl::flat_hash_map<int, std::unique_ptr<q2::proto::PlayerResults>>
      player_results;
  player_results.reserve(players_.size());
  for (int i = 0; i < 2; i++) {
    const int id = players_[i]->Id();
    player_results.emplace(id, absl::make_unique<q2::proto::PlayerResults>());
    player_results[id]->set_player_id(id);
  }

  for (int i = 0; i < results.size(); i += 2) {
    auto* mirrored_game_result =
        tournament_results->add_mirrored_game_results();
    mirrored_game_result->add_mirrored_games()->CopyFrom(*results[i]);
    mirrored_game_result->add_mirrored_games()->CopyFrom(*results[i + 1]);
    DoGameStats(results[i].get(), &player_results);
    DoGameStats(results[i + 1].get(), &player_results);
    DoPairStats(results[i].get(), results[i + 1].get(), &player_results);
  }
  for (const auto& player_result : player_results) {
    LOG(INFO) << "player_result: " << player_result.second->DebugString();
  }
}