#include "src/scrabble/rack.h"

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/scrabble/tiles.h"

class RackTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/scrabble/testdata/english_scrabble_tiles.textproto");
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(RackTest, Display) {
  Rack rack(tiles_->ToLetterString("RETAINS").value());
  std::stringstream ss;
  rack.Display(*tiles_, ss);
  EXPECT_EQ(ss.str(), "RETAINS");
}

TEST_F(RackTest, Subsets) {
  Rack rack(tiles_->ToLetterString("QIS").value());
  const auto& subsets = rack.Subsets(*tiles_);
  for (const auto& subset : subsets) {
    LOG(INFO) << "subset: {" << subset.first << ", "
              << tiles_->ToString(subset.second).value() << "}";
  }
  EXPECT_EQ(subsets.size(), 8);
  for (const auto& subset : {"", "I", "Q", "S", "IQ", "IS", "QS", "IQS"}) {
    auto letters = tiles_->ToLetterString(subset).value();
    auto product = tiles_->ToProduct(letters);
    auto result = subsets.find(product);
    EXPECT_TRUE(result != subsets.end());
    EXPECT_EQ(result->second, letters);
  }
}

TEST_F(RackTest, Subsets2) {
  Rack rack(tiles_->ToLetterString("AAB?").value());
  const auto& subsets = rack.Subsets(*tiles_);
  for (const auto& subset : subsets) {
    LOG(INFO) << "subset: {" << subset.first << ", "
              << tiles_->ToString(subset.second).value() << "}";
  }
  EXPECT_EQ(subsets.size(), 6);
  for (const auto& subset : {"", "A", "B", "AA", "AB", "AAB"}) {
    auto letters = tiles_->ToLetterString(subset).value();
    auto product = tiles_->ToProduct(letters);
    auto result = subsets.find(product);
    EXPECT_TRUE(result != subsets.end());
    EXPECT_EQ(result->second, letters);
  }
}

TEST_F(RackTest, Subsets3) {
  Rack rack(tiles_->ToLetterString("AAB??").value());
  const auto& subsets = rack.Subsets(*tiles_);
  for (const auto& subset : subsets) {
    LOG(INFO) << "subset: {" << subset.first << ", "
              << tiles_->ToString(subset.second).value() << "}";
  }
  EXPECT_EQ(subsets.size(), 6);
  for (const auto& subset : {"", "A", "B", "AA", "AB", "AAB"}) {
    auto letters = tiles_->ToLetterString(subset).value();
    auto product = tiles_->ToProduct(letters);
    auto result = subsets.find(product);
    EXPECT_TRUE(result != subsets.end());
    EXPECT_EQ(result->second, letters);
  }
}

TEST_F(RackTest, Subsets4) {
  Rack rack(tiles_->ToLetterString("ZANIEST").value());
  const auto& subsets = rack.Subsets(*tiles_);
  for (const auto& subset : subsets) {
    LOG(INFO) << "subset: {" << subset.first << ", "
              << tiles_->ToString(subset.second).value() << "}";
  }
  EXPECT_EQ(subsets.size(), 128);
}

TEST_F(RackTest, Subsets5) {
  Rack rack(tiles_->ToLetterString("PIZZAZZ").value());
  const auto& subsets = rack.Subsets(*tiles_);
  for (const auto& subset : subsets) {
    LOG(INFO) << "subset: {" << subset.first << ", "
              << tiles_->ToString(subset.second).value() << "}";
  }
  EXPECT_EQ(subsets.size(), 40);
}

TEST_F(RackTest, NumBlanks) {
  Rack no_blanks(tiles_->ToLetterString("RETAINS").value());
  Rack one_blank(tiles_->ToLetterString("RETAIN?").value());
  Rack two_blanks(tiles_->ToLetterString("QUACK??").value());
  EXPECT_EQ(no_blanks.NumBlanks(*tiles_), 0);
  EXPECT_EQ(one_blank.NumBlanks(*tiles_), 1);
  EXPECT_EQ(two_blanks.NumBlanks(*tiles_), 2);
}

TEST_F(RackTest, Counts1) {
  Rack nonempty(tiles_->ToLetterString("A").value());
  auto counts = nonempty.Counts();
  EXPECT_EQ(counts[L('A')], 1);
  for (Letter letter = L('B'); letter <= L('?'); ++letter) {
    EXPECT_EQ(counts[letter], 0);
  }
}

TEST_F(RackTest, Counts2) {
  Rack empty(tiles_->ToLetterString("").value());
  auto counts = empty.Counts();
  for (Letter letter = L('A'); letter <= L('?'); ++letter) {
    EXPECT_EQ(counts[letter], 0);
  }
}