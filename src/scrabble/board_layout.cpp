#include "src/scrabble/board_layout.h"

#include <fcntl.h>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "src/scrabble/board_spec.pb.h"

using ::google::protobuf::Arena;
using ::google::protobuf::TextFormat;
using ::google::protobuf::io::FileInputStream;

BoardLayout::BoardLayout(const std::string& filename)
    : proto_(LoadBoardSpec(filename).value()),
      symbol_map_(MakeSymbolMap(proto_)),
      square_symbols_(MakeSquareSymbols()),
      letter_multipliers_(MakeLetterMultipliers()),
      word_multipliers_(MakeWordMultipliers()) {}

absl::optional<q2::proto::BoardSpec> BoardLayout::LoadBoardSpec(
    const std::string& filename) {
  Arena arena;
  auto board_spec = Arena::CreateMessage<q2::proto::BoardSpec>(&arena);
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    LOG(ERROR) << "Error opening BoardSpec file " << filename;
    return absl::nullopt;
  }

  google::protobuf::io::FileInputStream fis(fd);
  fis.SetCloseOnDelete(true);

  if (!google::protobuf::TextFormat::Parse(&fis, board_spec)) {
    LOG(ERROR) << "Failed to parse BoardSpec file " << filename;
    return absl::nullopt;
  }
  return *board_spec;
}

absl::flat_hash_map<char32_t, const q2::proto::PremiumSquareDef*>
BoardLayout::MakeSymbolMap(const q2::proto::BoardSpec& proto) {
  absl::flat_hash_map<char32_t, const q2::proto::PremiumSquareDef*> ret;
  for (const auto& square : proto.premium_squares()) {
    const std::string& utf8 = square.fullwidth_symbol();
    std::u32string utf32 =
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
            .from_bytes(utf8);
    ret[utf32[0]] = &square;
  }
  //LOG(INFO) << "Loaded " << ret.size() << " premium squares";
  return ret;
}

namespace {
int Width(const std::string& utf8) {
  std::u32string utf32 =
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(
          utf8);
  return utf32.size();
}
}  // namespace

std::array<std::array<char32_t, 15>, 15> BoardLayout::MakeSquareSymbols()
    const {
  std::array<std::array<char32_t, 15>, 15> ret;
  const int height = proto_.premium_square_rows().size();
  if (height != 15) {
    LOG(FATAL) << "Needed 15 premium_squares_rows, have " << height;
  }
  const int width = Width(proto_.premium_square_rows(0));
  for (int i = 0; i < height; ++i) {
    const std::string& row = proto_.premium_square_rows(i);
    if (Width(row) != width) {
      LOG(FATAL) << "Row width mismatch: " << Width(row) << " vs " << width
                 << " for " << proto_.premium_square_rows(0) << " vs " << row;
    }
    std::u32string utf32 =
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
            .from_bytes(row);
    for (int j = 0; j < width; j++) {
      ret[i][j] = utf32[j];
    }
  }
  //LOG(INFO) << "Made square symbols";
  return ret;
}

std::array<std::array<int, 15>, 15> BoardLayout::MakeLetterMultipliers() const {
  std::array<std::array<int, 15>, 15> ret;
  const int height = proto_.premium_square_rows().size();
  if (height != 15) {
    LOG(FATAL) << "Needed 15 premium_squares_rows, have " << height;
  }
  const int width = Width(proto_.premium_square_rows(0));
  for (int i = 0; i < height; ++i) {
    const std::string& row = proto_.premium_square_rows(i);
    if (Width(row) != width) {
      LOG(FATAL) << "Row width mismatch: " << Width(row) << " vs " << width
                 << " for " << proto_.premium_square_rows(0) << " vs " << row;
    }
    std::u32string utf32 =
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
            .from_bytes(row);
    for (int j = 0; j < width; j++) {
      char32_t symbol = utf32[j];
      if (symbol_map_.count(symbol) == 0) {
        LOG(FATAL) << "Unexpected symbol " << symbol << " found at "
                   << "(" << i << ", " << j << ")";
      }
      const auto* square = symbol_map_.at(symbol);
      ret[i][j] = square->letter_multiplier();
    }
  }
  //LOG(INFO) << "Made letter multipliers";
  return ret;
}

std::array<std::array<int, 15>, 15> BoardLayout::MakeWordMultipliers() const {
  std::array<std::array<int, 15>, 15> ret;
  const int height = proto_.premium_square_rows().size();
  if (height != 15) {
    LOG(FATAL) << "Needed 15 premium_squares_rows, have " << height;
  }
  const int width = Width(proto_.premium_square_rows(0));
  for (int i = 0; i < height; ++i) {
    const std::string& row = proto_.premium_square_rows(i);
    if (Width(row) != width) {
      LOG(FATAL) << "Row width mismatch: " << Width(row) << " vs " << width
                 << " for " << proto_.premium_square_rows(0) << " vs " << row;
    }
    std::u32string utf32 =
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
            .from_bytes(row);
    for (int j = 0; j < width; j++) {
      char32_t symbol = utf32[j];
      if (symbol_map_.count(symbol) == 0) {
        LOG(FATAL) << "Unexpected symbol " << symbol << " found at "
                   << "(" << i << ", " << j << ")";
      }
      const auto* square = symbol_map_.at(symbol);
      ret[i][j] = square->word_multiplier();
    }
  }
  //LOG(INFO) << "Made word multipliers";
  return ret;
}

void BoardLayout::DisplayRow(const Board& board, int row_index,
                             const Tiles& tiles, std::ostream& os) const {
  const int display_row = row_index + 1;
  const std::string row_index_str = std::to_string(display_row);
  if (row_index_str.length() == 1) {
    os << " ";
  }
  os << row_index_str;
  for (int i = 0; i < 15; ++i) {
    const auto& letter = board.Row(row_index)[i];
    char32_t codepoint;
    if (letter == 0) {
      codepoint = square_symbols_[row_index][i];
    } else {
      codepoint = tiles.FullWidth(letter);
    }
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    const std::string utf8 = conv.to_bytes(codepoint);
    //const std::string utf8 = conv.to_bytes(&codepoint, &codepoint + 1);
    os << utf8;
  }
}

void BoardLayout::DisplayHeader(std::ostream& os) const {
  os << "  ";
  for (int i = 0; i < 15; ++i) {
    const char32_t codepoint = L'Ａ' + i;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    const std::string utf8 = conv.to_bytes(&codepoint, &codepoint + 1);
    os << utf8;
  }
}

void BoardLayout::DisplayBoard(const Board& board, const Tiles& tiles,
                               std::ostream& os) const {
  DisplayHeader(os);
  os << std::endl;
  for (int i = 0; i < 15; ++i) {
    DisplayRow(board, i, tiles, os);
    os << std::endl;
  }
}