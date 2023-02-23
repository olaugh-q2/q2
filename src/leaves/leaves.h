#ifndef SRC_LEAVES_LEAVES_H_
#define SRC_LEAVES_LEAVES_H_

#include "absl/container/flat_hash_map.h"
#include "glog/logging.h"
#include "src/scrabble/tiles.h"

class Leaves {
 public:
  static std::unique_ptr<Leaves> CreateFromCsv(const Tiles& tiles,
                                               const std::string& filename);

  float Value(uint64_t product) const {
    const auto it = leave_map_.find(product);
    if (it == leave_map_.end()) {
      LOG(ERROR) << "No leave value for product " << product;
      return 0.0;
    }
    return it->second;
  }

 private:
  absl::flat_hash_map<uint64_t, float> leave_map_;
};

#endif  // SRC_LEAVES_LEAVES_H_