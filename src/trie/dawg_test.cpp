#include "src/trie/dawg.h"

#include "absl/container/flat_hash_set.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DawgTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/trie/testdata/english_scrabble_tiles.textproto");
  }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(DawgTest, CreateFromTextfile) {
  // AIR, AIRPLANE, AIRWAY, AIRWAYS, HAS, HAT, HORSE, HOUSE, ZZZ
  const std::string input_filepath = "src/trie/testdata/simple.txt";
  auto dawg = Dawg::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(dawg, nullptr);

  auto root = dawg->Root();
  ASSERT_NE(root, nullptr);

  absl::flat_hash_set<int> node_indices;
  dawg->AddUniqueIndices(root, &node_indices);
  EXPECT_EQ(node_indices.size(), 27);

  EXPECT_FALSE(root->IsWord());

  const auto& root_children = root->ChildIndices();
  EXPECT_EQ(root_children.size(), 3);

  const auto* a = dawg->NodeAt(root_children[0]);
  EXPECT_EQ(a->Chars(), "A");
  EXPECT_FALSE(a->IsWord());
  const auto& a_children = a->ChildIndices();
  EXPECT_EQ(a_children.size(), 1);

  const auto* ai = dawg->NodeAt(a_children[0]);
  EXPECT_EQ(ai->Chars(), "I");
  EXPECT_FALSE(ai->IsWord());
  const auto& ai_children = ai->ChildIndices();
  EXPECT_EQ(ai_children.size(), 1);

  const auto* air = dawg->NodeAt(ai_children[0]);
  EXPECT_EQ(air->Chars(), "R");
  EXPECT_TRUE(air->IsWord());
  const auto& air_children = air->ChildIndices();
  EXPECT_EQ(air_children.size(), 2);

  const auto* airp = dawg->NodeAt(air_children[0]);
  EXPECT_EQ(airp->Chars(), "P");
  EXPECT_FALSE(airp->IsWord());
  const auto& airp_children = airp->ChildIndices();
  EXPECT_EQ(airp_children.size(), 1);

  const auto* airpl = dawg->NodeAt(airp_children[0]);
  EXPECT_EQ(airpl->Chars(), "L");
  EXPECT_FALSE(airpl->IsWord());
  const auto& airpl_children = airpl->ChildIndices();
  EXPECT_EQ(airpl_children.size(), 1);

  const auto* airpla = dawg->NodeAt(airpl_children[0]);
  EXPECT_EQ(airpla->Chars(), "A");
  EXPECT_FALSE(airpla->IsWord());
  const auto& airpla_children = airpla->ChildIndices();
  EXPECT_EQ(airpla_children.size(), 1);

  const auto* airplan = dawg->NodeAt(airpla_children[0]);
  EXPECT_EQ(airplan->Chars(), "N");
  EXPECT_FALSE(airplan->IsWord());
  const auto& airplan_children = airplan->ChildIndices();
  EXPECT_EQ(airplan_children.size(), 1);

  const auto* airplane = dawg->NodeAt(airplan_children[0]);
  EXPECT_EQ(airplane->Chars(), "E");
  EXPECT_TRUE(airplane->IsWord());
  const auto& airplane_children = airplane->ChildIndices();
  EXPECT_EQ(airplane_children.size(), 0);

  const auto* airw = dawg->NodeAt(air_children[1]);
  EXPECT_EQ(airw->Chars(), "W");
  EXPECT_FALSE(airw->IsWord());
  const auto& airw_children = airw->ChildIndices();
  EXPECT_EQ(airw_children.size(), 1);

  const auto* airwa = dawg->NodeAt(airw_children[0]);
  EXPECT_EQ(airwa->Chars(), "A");
  EXPECT_FALSE(airwa->IsWord());
  const auto& airwa_children = airwa->ChildIndices();
  EXPECT_EQ(airwa_children.size(), 1);

  const auto* airway = dawg->NodeAt(airwa_children[0]);
  EXPECT_EQ(airway->Chars(), "Y");
  EXPECT_TRUE(airway->IsWord());
  const auto& airway_children = airway->ChildIndices();
  EXPECT_EQ(airway_children.size(), 1);

  const auto* airways = dawg->NodeAt(airway_children[0]);
  EXPECT_EQ(airways->Chars(), "S");
  EXPECT_TRUE(airways->IsWord());
  const auto& airways_children = airways->ChildIndices();
  EXPECT_EQ(airways_children.size(), 0);

  const auto* h = dawg->NodeAt(root_children[1]);
  EXPECT_EQ(h->Chars(), "H");
  EXPECT_FALSE(h->IsWord());
  const auto& h_children = h->ChildIndices();
  EXPECT_EQ(h_children.size(), 2);

  const auto* ha = dawg->NodeAt(h_children[0]);
  EXPECT_EQ(ha->Chars(), "A");
  EXPECT_FALSE(ha->IsWord());
  const auto& ha_children = ha->ChildIndices();
  EXPECT_EQ(ha_children.size(), 2);

  const auto* has = dawg->NodeAt(ha_children[0]);
  EXPECT_EQ(has->Chars(), "S");
  EXPECT_TRUE(has->IsWord());
  const auto& has_children = has->ChildIndices();
  EXPECT_EQ(has_children.size(), 0);

  const auto* hat = dawg->NodeAt(ha_children[1]);
  EXPECT_EQ(hat->Chars(), "T");
  EXPECT_TRUE(hat->IsWord());
  const auto& hat_children = hat->ChildIndices();
  EXPECT_EQ(hat_children.size(), 0);

  const auto* ho = dawg->NodeAt(h_children[1]);
  EXPECT_EQ(ho->Chars(), "O");
  EXPECT_FALSE(ho->IsWord());
  const auto& ho_children = ho->ChildIndices();
  EXPECT_EQ(ho_children.size(), 2);

  const auto* hor = dawg->NodeAt(ho_children[0]);
  EXPECT_EQ(hor->Chars(), "R");
  EXPECT_FALSE(hor->IsWord());
  const auto& hor_children = hor->ChildIndices();
  EXPECT_EQ(hor_children.size(), 1);

  const auto* hors = dawg->NodeAt(hor_children[0]);
  EXPECT_EQ(hors->Chars(), "S");
  EXPECT_FALSE(hors->IsWord());
  const auto& hors_children = hors->ChildIndices();
  EXPECT_EQ(hors_children.size(), 1);

  const auto* horse = dawg->NodeAt(hors_children[0]);
  EXPECT_EQ(horse->Chars(), "E");
  EXPECT_TRUE(horse->IsWord());
  const auto& horse_children = horse->ChildIndices();
  EXPECT_EQ(horse_children.size(), 0);

  const auto* z = dawg->NodeAt(root_children[2]);
  EXPECT_EQ(z->Chars(), "Z");
  EXPECT_FALSE(z->IsWord());
  const auto& z_children = z->ChildIndices();
  EXPECT_EQ(z_children.size(), 1);

  const auto& zz = dawg->NodeAt(z_children[0]);
  EXPECT_EQ(zz->Chars(), "Z");
  EXPECT_FALSE(zz->IsWord());
  const auto& zz_children = zz->ChildIndices();
  EXPECT_EQ(zz_children.size(), 1);

  const auto* zzz = dawg->NodeAt(zz_children[0]);
  EXPECT_EQ(zzz->Chars(), "Z");
  EXPECT_TRUE(zzz->IsWord());
  const auto& zzz_children = zzz->ChildIndices();
  EXPECT_EQ(zzz_children.size(), 0);
}

TEST_F(DawgTest, MergeSingleChildrenIntoParents) {
  // AIR, AIRPLANE, AIRWAY, AIRWAYS, HAS, HAT, HORSE, HOUSE, ZZZ
  const std::string input_filepath = "src/trie/testdata/simple.txt";
  auto dawg = Dawg::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(dawg, nullptr);
  dawg->MergeSingleChildrenIntoParents();

  const auto& root = dawg->Root();
  ASSERT_NE(root, nullptr);

  absl::flat_hash_set<int> node_indices;
  dawg->AddUniqueIndices(root, &node_indices);
  EXPECT_EQ(node_indices.size(), 13);

  EXPECT_FALSE(root->IsWord());

  const auto& root_children = root->ChildIndices();
  EXPECT_EQ(root_children.size(), 3);

  const auto* air = dawg->NodeAt(root_children[0]);
  EXPECT_EQ(air->Chars(), "AIR");
  EXPECT_TRUE(air->IsWord());
  const auto& air_children = air->ChildIndices();
  EXPECT_EQ(air_children.size(), 2);

  const auto* airplane = dawg->NodeAt(air_children[0]);
  EXPECT_EQ(airplane->Chars(), "PLANE");
  EXPECT_TRUE(airplane->IsWord());
  const auto& airplane_children = airplane->ChildIndices();
  EXPECT_EQ(airplane_children.size(), 0);

  const auto* airway = dawg->NodeAt(air_children[1]);
  EXPECT_EQ(airway->Chars(), "WAY");
  EXPECT_TRUE(airway->IsWord());
  const auto& airway_children = airway->ChildIndices();
  EXPECT_EQ(airway_children.size(), 1);

  const auto* airways = dawg->NodeAt(airway_children[0]);
  EXPECT_EQ(airways->Chars(), "S");
  EXPECT_TRUE(airways->IsWord());
  const auto& airways_children = airways->ChildIndices();
  EXPECT_EQ(airways_children.size(), 0);

  const auto* h = dawg->NodeAt(root_children[1]);
  EXPECT_EQ(h->Chars(), "H");
  EXPECT_FALSE(h->IsWord());
  const auto& h_children = h->ChildIndices();
  EXPECT_EQ(h_children.size(), 2);

  const auto* ho = dawg->NodeAt(h_children[1]);
  EXPECT_EQ(ho->Chars(), "O");
  EXPECT_FALSE(ho->IsWord());
  const auto& ho_children = ho->ChildIndices();
  EXPECT_EQ(ho_children.size(), 2);

  const auto* horse = dawg->NodeAt(ho_children[0]);
  EXPECT_EQ(horse->Chars(), "RSE");
  EXPECT_TRUE(horse->IsWord());
  const auto& horse_children = horse->ChildIndices();
  EXPECT_EQ(horse_children.size(), 0);

  const auto* house = dawg->NodeAt(ho_children[1]);
  EXPECT_EQ(house->Chars(), "USE");
  EXPECT_TRUE(house->IsWord());
  const auto& house_children = house->ChildIndices();
  EXPECT_EQ(house_children.size(), 0);

  const auto* zzz = dawg->NodeAt(root_children[2]);
  EXPECT_EQ(zzz->Chars(), "ZZZ");
  EXPECT_TRUE(zzz->IsWord());
  const auto& zzz_children = zzz->ChildIndices();
  EXPECT_EQ(zzz_children.size(), 0);
}

TEST_F(DawgTest, MergeDuplicateSubtrees) {
  // AIR, AIRPLANE, AIRWAY, AIRWAYS, HAS, HAT, HORSE, HOUSE, ZZZ
  const std::string input_filepath = "src/trie/testdata/simple.txt";
  auto dawg = Dawg::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(dawg, nullptr);
  dawg->MergeSingleChildrenIntoParents();
  dawg->MergeDuplicateSubtrees();

  const auto& root = dawg->Root();
  ASSERT_NE(root, nullptr);

  absl::flat_hash_set<int> node_indices;
  dawg->AddUniqueIndices(root, &node_indices);
  EXPECT_EQ(node_indices.size(), 12);

  EXPECT_FALSE(root->IsWord());

  const auto& root_children = root->ChildIndices();
  EXPECT_EQ(root_children.size(), 3);

  const auto* air = dawg->NodeAt(root_children[0]);
  EXPECT_EQ(air->Chars(), "AIR");
  EXPECT_TRUE(air->IsWord());
  const auto& air_children = air->ChildIndices();
  EXPECT_EQ(air_children.size(), 2);

  const auto* airplane = dawg->NodeAt(air_children[0]);
  EXPECT_EQ(airplane->Chars(), "PLANE");
  EXPECT_TRUE(airplane->IsWord());
  const auto& airplane_children = airplane->ChildIndices();
  EXPECT_EQ(airplane_children.size(), 0);

  const auto* airway = dawg->NodeAt(air_children[1]);
  EXPECT_EQ(airway->Chars(), "WAY");
  EXPECT_TRUE(airway->IsWord());
  const auto& airway_children = airway->ChildIndices();
  EXPECT_EQ(airway_children.size(), 1);

  const auto* airways = dawg->NodeAt(airway_children[0]);
  EXPECT_EQ(airways->Chars(), "S");
  EXPECT_TRUE(airways->IsWord());
  const auto& airways_children = airways->ChildIndices();
  EXPECT_EQ(airways_children.size(), 0);

  const auto* h = dawg->NodeAt(root_children[1]);
  EXPECT_EQ(h->Chars(), "H");
  EXPECT_FALSE(h->IsWord());
  const auto& h_children = h->ChildIndices();
  EXPECT_EQ(h_children.size(), 2);

  const auto* ho = dawg->NodeAt(h_children[1]);
  EXPECT_EQ(ho->Chars(), "O");
  EXPECT_FALSE(ho->IsWord());
  const auto& ho_children = ho->ChildIndices();
  EXPECT_EQ(ho_children.size(), 2);

  const auto* horse = dawg->NodeAt(ho_children[0]);
  EXPECT_EQ(horse->Chars(), "RSE");
  EXPECT_TRUE(horse->IsWord());
  const auto& horse_children = horse->ChildIndices();
  EXPECT_EQ(horse_children.size(), 0);

  const auto* house = dawg->NodeAt(ho_children[1]);
  EXPECT_EQ(house->Chars(), "USE");
  EXPECT_TRUE(house->IsWord());
  const auto& house_children = house->ChildIndices();
  EXPECT_EQ(house_children.size(), 0);

  const auto* zzz = dawg->NodeAt(root_children[2]);
  EXPECT_EQ(zzz->Chars(), "ZZZ");
  EXPECT_TRUE(zzz->IsWord());
  const auto& zzz_children = zzz->ChildIndices();
  EXPECT_EQ(zzz_children.size(), 0);
}
