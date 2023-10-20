#include "src/rack_table/magpie_rack.h"

#include "glog/logging.h"

std::vector<MagpieRack> GetSubselections(const MagpieRack& bag, int max_size) {
  std::vector<MagpieRack> subselections;
  CompleteSubselections(bag, max_size, 0, MagpieRack(), &subselections);
  return subselections;
}

void CompleteSubselections(const MagpieRack& bag, int max_size, int position,
                           const MagpieRack chosen,
                           std::vector<MagpieRack>* subselections) {
  if (position == bag.counts.size()) {
    if (chosen.num_elements <= max_size) {
      subselections->push_back(chosen);
    }
    return;
  }
  for (int i = 0; i <= bag.counts[position]; ++i) {
    MagpieRack new_chosen = chosen;
    new_chosen.counts[position] = i;
    new_chosen.num_elements += i;
    if (new_chosen.num_elements > max_size) {
      break;
    }
    CompleteSubselections(bag, max_size, position + 1, new_chosen,
                          subselections);
  }
}

MagpieRack MagpieRackFromString(const std::string& letters) {
  MagpieRack rack;
  rack.counts.fill(0);
  rack.num_elements = 0;
  for (const char& letter : letters) {
    if (letter == '?') {
      rack.counts[27]++;
    } else {
      rack.counts[letter - 'A' + 1]++;
    }
    rack.num_elements++;
  }
  return rack;
}

std::string MagpieRackToString(const MagpieRack& magpie_rack) {
  std::string ret;
  for (int i = 1; i < magpie_rack.counts.size(); ++i) {
    for (int j = 0; j < magpie_rack.counts[i]; ++j) {
      if (i == 27) {
        ret.push_back('?');
      } else {
        ret.push_back('A' + i - 1);
      }
    }
  }
  return ret;
}

uint64_t MagpieRackToUint64(const MagpieRack& magpie_rack) {
  CHECK(magpie_rack.num_elements <= 8);
  uint64_t ret = 0;
  for (int i = 1; i < magpie_rack.counts.size(); ++i) {
    const int num_this = magpie_rack.counts[i];
    for (int j = 0; j < num_this; ++j) {
      ret = (ret << 5) | i;
    }
  }
  return ret;
}

std::vector<MagpieRack> Blankify(const MagpieRack& rack) {
  std::vector<MagpieRack> blanked;
  for (int i = 1; i <= 26; ++i) {
    if (rack.counts[i] > 0) {
      MagpieRack new_rack = rack;
      new_rack.counts[i]--;
      new_rack.counts[27]++;
      blanked.push_back(new_rack);
    }
  }
  return blanked;
}

std::vector<MagpieRack> AddTile(const MagpieRack& rack) {
  std::vector<MagpieRack> with_extra_tile;
  for (int i = 1; i <= 27; i++) {
    auto new_rack = rack;
    new_rack.counts[i]++;
    new_rack.num_elements++;
    with_extra_tile.push_back(new_rack);
  }
  return with_extra_tile;
}

absl::flat_hash_map<uint64_t, std::array<int, 8>> MakeRackWordSizesMap(
    const std::vector<std::string>& words, const MagpieRack& bag, int max_size,
    int max_playthrough) {
  absl::flat_hash_map<uint64_t, std::array<int, 8>> map;
  const std::vector<MagpieRack> subselections = GetSubselections(bag, max_size);
  for (const auto& s : subselections) {
    if (s.num_elements == 0) {
      continue;
    }
    const uint64_t key = MagpieRackToUint64(s);
    map[key].fill(0);
  }
  for (const auto& word : words) {
    std::vector<MagpieRack> subselections;
    const MagpieRack word_rack = MagpieRackFromString(word);
    if (word_rack.num_elements > max_size + max_playthrough) {
      continue;
    }
    std::vector<MagpieRack> natural_subselections =
        GetSubselections(word_rack, max_size);
    for (const auto& s : natural_subselections) {
      subselections.push_back(s);
      const auto blanked = Blankify(s);
      for (const auto& b : blanked) {
        subselections.push_back(b);
        const auto double_blanked = Blankify(b);
        for (const auto& db : double_blanked) {
          subselections.push_back(db);
        }
      }
    }
    for (const auto& s : subselections) {
      if (s.num_elements == 0) {
        continue;
      }
      int played_through = word_rack.num_elements - s.num_elements;
      if (played_through > max_playthrough) {
        continue;
      }
      const uint64_t key = MagpieRackToUint64(s);
      if (map.find(key) == map.end()) {
        //LOG(INFO) << "impossible rack: " << MagpieRackToString(s);
        continue;
      }
      if (map[key][played_through] < s.num_elements) {
        map[key][played_through] = s.num_elements;
      }
    }
  }
  for (int num_elements = 1; num_elements < max_size; num_elements++) {
    for (const auto& s : subselections) {
      const uint64_t key1 = MagpieRackToUint64(s);
      if (s.num_elements != num_elements) {
        continue;
      }
      if (map.find(key1) == map.end()) {
        //LOG(INFO) << "impossible rack: " << MagpieRackToString(s);
        continue;
      }
      const std::vector<MagpieRack> with_extra_tile = AddTile(s);
      for (const auto& s_plus_tile : with_extra_tile) {
        const uint64_t key2 = MagpieRackToUint64(s_plus_tile);
        if (map.find(key2) == map.end()) {
          //LOG(INFO) << "impossible rack: " << MagpieRackToString(s_plus_tile);
          continue;
        }
        for (int i = 0; i <= max_playthrough; ++i) {
          if (map[key2][i] < map[key1][i]) {
            map[key2][i] = map[key1][i];
          }
        }
      }
    }
  }
  return map;
}