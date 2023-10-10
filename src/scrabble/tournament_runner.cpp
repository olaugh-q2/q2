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

namespace {
void DoGameStats(
    const q2::proto::GameResult* game, int thread_index, int pair_index,
    std::vector<std::unique_ptr<q2::proto::PlayerResults>>* player_results) {
  int i0 = thread_index * 2;
  int i1 = thread_index * 2 + 1;
  if (pair_index % 2 == 1) {
    std::swap(i0, i1);
  }
  auto* r0 = (*player_results)[i0].get();
  auto* r1 = (*player_results)[i1].get();
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
  r0->set_total_time_used_micros(r0->total_time_used_micros() +
                                 game->micros_used(0));
  r1->set_total_time_used_micros(r1->total_time_used_micros() +
                                 game->micros_used(1));
  r0->set_total_time_remaining_micros(r0->total_time_remaining_micros() +
                                      game->micros_remaining(0));
  r1->set_total_time_remaining_micros(r1->total_time_remaining_micros() +
                                      game->micros_remaining(1));
}

void DoPairStats(
    const q2::proto::GameResult* g0, const q2::proto::GameResult* g1,
    int thread_index,
    std::vector<std::unique_ptr<q2::proto::PlayerResults>>* player_results) {
  int i0 = thread_index * 2;
  int i1 = thread_index * 2 + 1;
  auto* r0 = (*player_results)[i0].get();
  auto* r1 = (*player_results)[i1].get();
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
  int p0diff = g0->player_scores(0) - g0->player_scores(1) +
               g1->player_scores(1) - g1->player_scores(0);
  if (p0diff != 0) {
    /*
  if ((p0diff < -0) || (p0diff > 0)) {
  LOG(INFO) << "p0diff: " << p0diff << " g0: " << std::endl
     << g0->DebugString() << std::endl
     << "g1: " << std::endl
     << g1->DebugString();
  }
  */
    r0->set_total_mirrored_difference(r0->total_mirrored_difference() + p0diff);
    r1->set_total_mirrored_difference(r1->total_mirrored_difference() - p0diff);
    r0->set_mirrored_with_difference(r0->mirrored_with_difference() + 1);
    r1->set_mirrored_with_difference(r1->mirrored_with_difference() + 1);
  }
  r0->set_mirrored_score_difference_sum_of_squares(
      r0->mirrored_score_difference_sum_of_squares() + p0diff * p0diff);
  r1->set_mirrored_score_difference_sum_of_squares(
      r1->mirrored_score_difference_sum_of_squares() + p0diff * p0diff);
}
}  // namespace

void TournamentRunner::RunGames(
    const Tiles& tiles, const BoardLayout& board_layout, int thread_index,
    int start_index, int num_pairs,
    std::vector<std::unique_ptr<q2::proto::PlayerResults>>* player_results)
    const {
  absl::BitGen gen;
  std::vector<Player*> players;
  for (int i = 0; i < 2; i++) {
    auto& player = players_[thread_index * 2 + i];
    players.push_back(player.get());
  }
  for (int i = 0; i < num_pairs; i++) {
    // LOG(INFO) << "Running pair " << i << " on thread " << thread_index;
    Bag bag(tiles);
    bag.Shuffle(gen);
    std::vector<uint16_t> exchange_insertion_dividends;
    for (int i = 0; i < 1000; ++i) {
      exchange_insertion_dividends.push_back(
          absl::Uniform<uint16_t>(gen, 0, 65535));
    }
    std::vector<std::unique_ptr<q2::proto::GameResult>> results;
    results.resize(2);
    for (int j = 0; j < 2; j++) {
      // LOG(INFO) << "i: " << i << " j: " << j;
      Game game(board_layout, players, tiles, absl::Minutes(25), i);
      game.CreateInitialPosition(bag, exchange_insertion_dividends);
      game.FinishWithComputerPlayers();
      results[j] = absl::make_unique<q2::proto::GameResult>();
      game.WriteProto(results[j].get());
      std::reverse(players.begin(), players.end());
    }
    DoGameStats(results[0].get(), thread_index, 0, player_results);
    DoGameStats(results[1].get(), thread_index, 1, player_results);
    DoPairStats(results[0].get(), results[1].get(), thread_index,
                player_results);
    auto* cf = ComponentFactory::GetInstance();
    auto* ordering_provider = cf->GetTileOrderingProvider();
    ordering_provider->RemoveGame(i);

    /*
    if ((thread_index == 0) && (i % 10) == 0) {
      int thousandths = (1000 * i) / num_pairs;
      LOG(INFO) << "Progress: " << thousandths << "/1000";
    }
    */
  }
}

void TournamentRunner::Run(q2::proto::TournamentResults* tournament_results) {
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

  const auto& singletons = spec_.singleton_components();
  ComponentFactory::GetInstance()->CreateSingletonComponents(singletons);

  Arena arena;
  std::vector<std::unique_ptr<q2::proto::PlayerResults>> player_results;

  for (int i = 0; i < spec_.number_of_threads(); ++i) {
    for (const auto& player_spec : spec_.players()) {
      auto player = ComponentFactory::CreatePlayerFromConfig(player_spec);
      CHECK(player != nullptr);
      players_.push_back(std::move(player));
    }
  }
  player_results.reserve(players_.size());
  for (int i = 0; i < players_.size(); i++) {
    player_results.push_back(absl::make_unique<q2::proto::PlayerResults>());
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
  LOG(INFO) << "Starting games...";
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
                          pairs_to_run, &player_results]() {
      RunGames(tiles, board_layout, i, start_index * 2, pairs_to_run,
               &player_results);
    });
  }
  // Wait for all threads to finish
  for (auto& thread : threads) {
    thread.join();
  }
  LOG(INFO) << "Finished games!";
  std::vector<std::unique_ptr<q2::proto::PlayerResults>> aggregated_results;
  aggregated_results.resize(2);
  for (int i = 0; i < 2; i++) {
    aggregated_results[i] = absl::make_unique<q2::proto::PlayerResults>();
  }
  for (int i = 0; i < player_results.size(); i++) {
    // LOG(INFO) << "Player " << i
    //           << " results: " << player_results[i]->DebugString();
    int target_index = i % 2;
    auto* p = aggregated_results[target_index].get();
    p->set_player_id(1 + target_index);
    p->set_games_started(p->games_started() +
                         player_results[i]->games_started());
    p->set_num_wins(p->num_wins() + player_results[i]->num_wins());
    p->set_num_losses(p->num_losses() + player_results[i]->num_losses());
    p->set_num_draws(p->num_draws() + player_results[i]->num_draws());
    p->set_total_score(p->total_score() + player_results[i]->total_score());
    p->set_total_opponent_score(p->total_opponent_score() +
                                player_results[i]->total_opponent_score());
    p->set_score_difference_sum_of_squares(
        p->score_difference_sum_of_squares() +
        player_results[i]->score_difference_sum_of_squares());
    p->set_total_time_used_micros(p->total_time_used_micros() +
                                  player_results[i]->total_time_used_micros());
    p->set_total_time_remaining_micros(
        p->total_time_remaining_micros() +
        player_results[i]->total_time_remaining_micros());
    p->set_total_mirrored_difference(
        p->total_mirrored_difference() +
        player_results[i]->total_mirrored_difference());
    p->set_mirrored_sweeps(p->mirrored_sweeps() +
                           player_results[i]->mirrored_sweeps());
    p->set_mirrored_3quart(p->mirrored_3quart() +
                           player_results[i]->mirrored_3quart());
    p->set_mirrored_splits(p->mirrored_splits() +
                           player_results[i]->mirrored_splits());
    p->set_mirrored_1quart(p->mirrored_1quart() +
                           player_results[i]->mirrored_1quart());
    p->set_mirrored_sombreros(p->mirrored_sombreros() +
                              player_results[i]->mirrored_sombreros());
    p->set_mirrored_with_difference(
        p->mirrored_with_difference() +
        player_results[i]->mirrored_with_difference());
    p->set_mirrored_score_difference_sum_of_squares(
        p->mirrored_score_difference_sum_of_squares() +
        player_results[i]->mirrored_score_difference_sum_of_squares());
  }
  for (const auto& player_result : aggregated_results) {
    LOG(INFO) << "player_result: " << player_result->DebugString();
  }
  std::vector<std::unique_ptr<q2::proto::PlayerAverages>> averages;
  ComputeAverages(aggregated_results, &averages);
  for (const auto& average : averages) {
    tournament_results->add_player_averages()->CopyFrom(*average);
  }
}

void TournamentRunner::ComputeAverages(
    const std::vector<std::unique_ptr<q2::proto::PlayerResults>>& results,
    std::vector<std::unique_ptr<q2::proto::PlayerAverages>>* averages) const {
  averages->reserve(results.size());
  for (const auto& result : results) {
    averages->emplace_back(absl::make_unique<q2::proto::PlayerAverages>());
    auto& a = averages->back();
    a->set_player_id(result->player_id());
    const int num_games =
        result->num_wins() + result->num_losses() + result->num_draws();
    a->set_average_score(static_cast<float>(result->total_score()) / num_games);
    a->set_average_opponent_score(
        static_cast<float>(result->total_opponent_score()) / num_games);
    const int64_t score_diff =
        result->total_score() - result->total_opponent_score();
    a->set_average_score_difference(static_cast<float>(score_diff) / num_games);
    a->set_score_difference_sd(
        sqrt((result->score_difference_sum_of_squares() -
              (score_diff * score_diff / static_cast<float>(num_games))) /
             static_cast<float>(num_games - 1)));
    a->set_average_time_remaining_micros(
        static_cast<float>(result->total_time_remaining_micros()) / num_games);
    a->set_average_time_used_micros(
        static_cast<float>(result->total_time_used_micros()) / num_games);

    const int mdiff_games = result->mirrored_with_difference();
    // LOG(INFO) << "mdiff_games: " << mdiff_games;
    const int mirrored_diff = result->total_mirrored_difference();
    // LOG(INFO) << "mirrored_diff: " << mirrored_diff;
    if (mdiff_games > 1) {
      a->set_mirrored_score_difference_sd(sqrt(
          (result->mirrored_score_difference_sum_of_squares() -
           (mirrored_diff * mirrored_diff / static_cast<float>(mdiff_games))) /
          static_cast<float>(mdiff_games - 1)));

      const float z = 2.576;  // 99% confidence
      const float mirrored_error = z * a->mirrored_score_difference_sd() /
                                   sqrt(static_cast<float>(mdiff_games));
      a->set_mirrored_confidence_lower_bound(a->average_score_difference() -
                                             mirrored_error);
      a->set_mirrored_confidence_upper_bound(a->average_score_difference() +
                                             mirrored_error);
    }
  }
}