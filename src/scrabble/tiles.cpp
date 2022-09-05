#include "src/scrabble/tiles.h"

#include <fcntl.h>

#include <iterator>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "src/scrabble/primes.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles_spec.pb.h"

using ::google::protobuf::Arena;
using ::google::protobuf::TextFormat;
using ::google::protobuf::io::FileInputStream;

Tiles::Tiles(const std::string& filename)
    : proto_(LoadTilesSpec(filename).value()),
      fullwidth_symbols_(MakeFullwidthSymbols()),
      distribution_(DistributionFromProto(proto_)),
      blank_index_(FindBlankIndex(proto_)),
      primes_(TilePrimes(Primes::FirstNPrimes(blank_index_), PrimeIndices())) {}

absl::optional<q2::proto::TilesSpec> Tiles::LoadTilesSpec(
    const std::string& filename) {
  Arena arena;
  auto tiles_spec = Arena::CreateMessage<q2::proto::TilesSpec>(&arena);
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    LOG(ERROR) << "Error opening TilesSpec file " << filename;
    return absl::nullopt;
  }

  google::protobuf::io::FileInputStream fis(fd);
  fis.SetCloseOnDelete(true);

  if (!google::protobuf::TextFormat::Parse(&fis, tiles_spec)) {
    LOG(ERROR) << "Failed to parse TilesSpec file " << filename;
    return absl::nullopt;
  }
  return *tiles_spec;
}

std::array<char32_t, 32> Tiles::MakeFullwidthSymbols() const {
  std::array<char32_t, 32> ret;
  for (int i = 0; i < proto_.tiles().size(); ++i) {
    const auto& tile = proto_.tiles(i);
    const int letter = i + 1;
    const std::string& utf8 = tile.natural_fullwidth();
    std::u32string utf32 =
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
            .from_bytes(utf8);            
    ret[letter] = utf32[0];
  }
  return ret;
}

std::array<int, 32> Tiles::DistributionFromProto(
    const q2::proto::TilesSpec& proto) {
  std::array<int, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (int i = 0; i < proto.tiles_size(); ++i) {
    const auto& tile = proto.tiles(i);
    ret[1 + i] = tile.count();
  }
  return ret;
}

int Tiles::FindBlankIndex(const q2::proto::TilesSpec& proto) {
  for (int i = 0; i < proto.tiles_size(); ++i) {
    const q2::proto::TileSpec& tile = proto.tiles(i);
    if (tile.tile_type() == q2::proto::TileSpec_TileType_BLANK) {
      return 1 + i;
    }
  }
  LOG(WARNING)
      << "No blank tile found in TilesSpec, using index 1+tiles.size(): ("
      << 1+proto.tiles_size() << ")";
  return 1+proto.tiles_size();
}

absl::optional<Letter> Tiles::CharToNumber(char c) const {
  LOG(INFO) << "CharToNumber(" << c << ")";
  if (c == '?') {
    LOG(INFO) << "returning " << blank_index_;
    return blank_index_;
  } else if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 1;
  } else if (c >= 'a' && c <= 'z') {
    LOG(INFO) << "returning " << c - 'a' + 1 + blank_index_;
    return c - 'a' + 1 + blank_index_;
  }
  LOG(ERROR) << "Could not convert character '" << c << " ("
             << static_cast<int>(c) << ")' to number";
  return absl::nullopt;
}

int Tiles::Count(Letter letter) const { return distribution_[letter]; }

absl::optional<char> Tiles::NumberToChar(Letter letter) const {
  if (letter == blank_index_) {
    return '?';
  } else if (letter > blank_index_ && letter <= blank_index_ + 26) {
    return (letter - blank_index_) + 'a' - 1;
  } else if (letter >= 1 && letter <= 26) {
    return letter + 'A' - 1;
  }
  LOG(ERROR) << "Could not convert number " << letter << " to character";
  return absl::nullopt;
}

absl::optional<LetterString> Tiles::ToLetterString(const std::string& s) const {
  LetterString ret;
  for (size_t i = 0; i < s.size(); i++) {
    const char c = s[i];
    auto number = CharToNumber(c);
    if (number.has_value()) {
      ret.push_back(number.value());
    } else {
      return absl::nullopt;
    }
  }
  return ret;
}

absl::optional<std::string> Tiles::ToString(const LetterString& s) const {
  std::string ret;
  for (char n : s) {
    auto c = NumberToChar(n);
    if (c.has_value()) {
      ret.push_back(c.value());
    } else {
      return absl::nullopt;
    }
  }
  return ret;
}

LetterString Tiles::Unblank(const LetterString& s) const {
  LetterString ret;
  for (Letter letter : s) {
    if (letter > blank_index_) {
      ret.push_back(letter - blank_index_);
    } else {
      ret.push_back(letter);
    }
  }
  return ret;
}

std::array<uint64_t, 32> Tiles::TilePrimes(
    const std::array<uint64_t, 32>& primes,
    const std::array<int, 32>& indices) {
  std::array<uint64_t, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (size_t i = 0; i < indices.size(); i++) {
    const int index = indices[i];
    if (index != 0) {
      ret[i] = primes[index];
    }
  }
  return ret;
}

std::array<int, 32> Tiles::PrimeIndices() {
  std::vector<std::pair<int, int>> counts;
  for (size_t i = 0; i < distribution_.size(); i++) {
    if (distribution_[i] > 0) {
      counts.push_back(std::make_pair(distribution_[i], i));
    }
  }
  std::sort(std::begin(counts), std::end(counts),
            [this](const std::pair<int, int>& a, const std::pair<int, int>& b) {
              // Blank is last regardless of count. It gets the biggest prime
              // because the maps containing large multisets don't have blanks
              // in their keys. I believe only maps for leave values and other
              // things with 7 or fewer tiles will need to contain blanks.
              if (a.second == this->blank_index_) {
                return false;
              }
              if (b.second == this->blank_index_) {
                return true;
              }
              if (a.first == b.first) {
                return a.second < b.second;
              }
              return a.first > b.first;
            });

  std::array<int, 32> ret;
  std::fill(std::begin(ret), std::end(ret), 0);
  for (size_t i = 0; i < counts.size(); i++) {
    ret[counts[i].second] = i + 1;
  }
  return ret;
}

absl::uint128 Tiles::ToProduct(const LetterString& s) const {
  absl::uint128 ret = 1;
  for (char c : s) {
    ret *= primes_[c];
  }
  return ret;
}