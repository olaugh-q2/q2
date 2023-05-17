#ifndef SRC_LEAVES_LEAVES_H
#define SRC_LEAVES_LEAVES_H

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "src/scrabble/tiles.h"

class Leaves {
 public:
  static std::unique_ptr<Leaves> CreateFromCsv(const Tiles& tiles,
                                               const std::string& filename);
  static std::unique_ptr<Leaves> CreateFromBinaryFile(
      const Tiles& tiles, const std::string& filename);
  absl::Status WriteToBinaryFile(const std::string& filename) const;

  float Value(uint64_t product) const;

 private:
  bool WriteToOstream(std::ostream& os) const;

  absl::flat_hash_map<uint64_t, float> leave_map_;
};

inline float Leaves::Value(uint64_t product) const {
  const auto it = leave_map_.find(product);
  /*
  if (it == leave_map_.end()) {
    LOG(ERROR) << "No leave value for product " << product;
    return 0.0;
  }
  */
  return it->second;
}
#endif  // SRC_LEAVES_LEAVES_H