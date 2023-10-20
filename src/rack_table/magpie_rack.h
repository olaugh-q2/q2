#ifndef SRC_RACK_TABLE_MAGPIE_RACK_H
#define SRC_RACK_TABLE_MAGPIE_RACK_H

#include <array>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"

typedef struct MagpieRack {
  std::array<int, 32> counts;
  int num_elements;
} MagpieRack;

std::vector<MagpieRack> GetSubselections(const MagpieRack& bag, int max_size);
void CompleteSubselections(const MagpieRack& bag, int max_size, int position,
                           const MagpieRack chosen,
                           std::vector<MagpieRack>* subselections);
MagpieRack MagpieRackFromString(const std::string& letters);
std::string MagpieRackToString(const MagpieRack& rack);
uint64_t MagpieRackToUint64(const MagpieRack& rack);

absl::flat_hash_map<uint64_t, std::array<int, 8>> MakeRackWordSizesMap(
    const std::vector<std::string>& words, const MagpieRack& bag, int max_size,
    int max_playthrough);

std::vector<MagpieRack> AddTile(const MagpieRack& rack);
#endif  // SRC_RACK_TABLE_MAGPIE_RACK_H