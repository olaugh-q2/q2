#ifndef SRC_SCRABBLE_BOARD_LAYOUT_H_
#define SRC_SCRABBLE_BOARD_LAYOUT_H_

#include <array>
#include <codecvt>
#include <locale>
#include <ostream>

#include "absl/container/flat_hash_map.h"
#include "absl/types/optional.h"
#include "src/scrabble/board.h"
#include "src/scrabble/board_spec.pb.h"

class BoardLayout {
 public:
  BoardLayout(const std::string& filename);

  void DisplayRow(const Board& board, int row_index, const Tiles& tiles,
                  std::ostream& os) const;
  void DisplayHeader(std::ostream& os) const;
  void DisplayTopBorder(std::ostream& os) const;
  void DisplayBottomBorder(std::ostream& os) const;
  void DisplayBoard(const Board& board, const Tiles& tiles, std::ostream& os) const;

 private:
  FRIEND_TEST(BoardLayoutTest, Construction);

  absl::optional<q2::proto::BoardSpec> LoadBoardSpec(
      const std::string& filename);

  absl::flat_hash_map<char32_t, const q2::proto::PremiumSquareDef*>
  MakeSymbolMap(const q2::proto::BoardSpec& proto);

  std::array<std::array<char32_t, 15>, 15> MakeSquareSymbols() const;
  std::array<std::array<int, 15>, 15> MakeLetterMultipliers() const;
  std::array<std::array<int, 15>, 15> MakeWordMultipliers() const;

  const q2::proto::BoardSpec proto_;
  const absl::flat_hash_map<char32_t, const q2::proto::PremiumSquareDef*>
      symbol_map_;

  const std::array<std::array<char32_t, 15>, 15> square_symbols_;
  const std::array<std::array<int, 15>, 15> letter_multipliers_;
  const std::array<std::array<int, 15>, 15> word_multipliers_;
};

#endif  // SRC_SCRABBLE_BOARD_LAYOUT_H_