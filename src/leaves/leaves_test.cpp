#include "src/leaves/leaves.h"

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class LeavesTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/leaves/testdata/english_scrabble_tiles.textproto");
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(LeavesTest, CreateFromCsv) {
  const std::string input_filepath = "src/leaves/testdata/macondo.csv";
  auto leaves = Leaves::CreateFromCsv(*tiles_, input_filepath);
  ASSERT_NE(leaves, nullptr);
  EXPECT_FLOAT_EQ(leaves->Value(1), 0.0);
  uint64_t i_product = tiles_->Prime(L('I'));
  EXPECT_FLOAT_EQ(leaves->Value(i_product), 0.7403481220894292);
  EXPECT_FLOAT_EQ(leaves->Value(1009 /* prime */), 0.0 /* error */);
  uint64_t anise_blank_product =
      static_cast<uint64_t>(tiles_->ToProduct(LS("AEINS?")));
  EXPECT_FLOAT_EQ(leaves->Value(anise_blank_product), 38.35431501026073);
}