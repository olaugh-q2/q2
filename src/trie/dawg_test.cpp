#include "src/trie/dawg.h"

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
  EXPECT_FALSE(root->IsWord());

  const auto& root_children = root->Children();
  EXPECT_EQ(root_children.size(), 3);

  const auto& a = root_children[0];
  EXPECT_EQ(a.c(), 'A');
  EXPECT_FALSE(a.IsWord());
  const auto& a_children = a.Children();
  EXPECT_EQ(a_children.size(), 1);

  const auto& ai = a_children[0];
  EXPECT_EQ(ai.c(), 'I');
  EXPECT_FALSE(ai.IsWord());
  const auto& ai_children = ai.Children();
  EXPECT_EQ(ai_children.size(), 1);

  const auto& air = ai_children[0];
  EXPECT_EQ(air.c(), 'R');
  EXPECT_TRUE(air.IsWord());
  const auto& air_children = air.Children();
  EXPECT_EQ(air_children.size(), 2);

  const auto& airp = air_children[0];
  EXPECT_EQ(airp.c(), 'P');
  EXPECT_FALSE(airp.IsWord());
  const auto& airp_children = airp.Children();
  EXPECT_EQ(airp_children.size(), 1);

  const auto& airpl = airp_children[0];
  EXPECT_EQ(airpl.c(), 'L');
  EXPECT_FALSE(airpl.IsWord());
  const auto& airpl_children = airpl.Children();
  EXPECT_EQ(airpl_children.size(), 1);

  const auto& airpla = airpl_children[0];
  EXPECT_EQ(airpla.c(), 'A');
  EXPECT_FALSE(airpla.IsWord());
  const auto& airpla_children = airpla.Children();
  EXPECT_EQ(airpla_children.size(), 1);

  const auto& airplan = airpla_children[0];
  EXPECT_EQ(airplan.c(), 'N');
  EXPECT_FALSE(airplan.IsWord());
  const auto& airplan_children = airplan.Children();
  EXPECT_EQ(airplan_children.size(), 1);

  const auto& airplane = airplan_children[0];
  EXPECT_EQ(airplane.c(), 'E');
  EXPECT_TRUE(airplane.IsWord());
  const auto& airplane_children = airplane.Children();
  EXPECT_EQ(airplane_children.size(), 0);

  const auto& airw = air_children[1];
  EXPECT_EQ(airw.c(), 'W');
  EXPECT_FALSE(airw.IsWord());
  const auto& airw_children = airw.Children();
  EXPECT_EQ(airw_children.size(), 1);

  const auto& airwa = airw_children[0];
  EXPECT_EQ(airwa.c(), 'A');
  EXPECT_FALSE(airwa.IsWord());
  const auto& airwa_children = airwa.Children();
  EXPECT_EQ(airwa_children.size(), 1);

  const auto& airway = airwa_children[0];
  EXPECT_EQ(airway.c(), 'Y');
  EXPECT_TRUE(airway.IsWord());
  const auto& airway_children = airway.Children();
  EXPECT_EQ(airway_children.size(), 1);

  const auto& airways = airway_children[0];
  EXPECT_EQ(airways.c(), 'S');
  EXPECT_TRUE(airways.IsWord());
  const auto& airways_children = airways.Children();
  EXPECT_EQ(airways_children.size(), 0);

  const auto& h = root_children[1];
  EXPECT_EQ(h.c(), 'H');
  EXPECT_FALSE(h.IsWord());
  const auto& h_children = h.Children();
  EXPECT_EQ(h_children.size(), 2);

    const auto& ha = h_children[0];
    EXPECT_EQ(ha.c(), 'A');
    EXPECT_FALSE(ha.IsWord());
    const auto& ha_children = ha.Children();
    EXPECT_EQ(ha_children.size(), 2);

    const auto& has = ha_children[0];
    EXPECT_EQ(has.c(), 'S');
    EXPECT_TRUE(has.IsWord());
    const auto& has_children = has.Children();
    EXPECT_EQ(has_children.size(), 0);

    const auto& hat = ha_children[1];
    EXPECT_EQ(hat.c(), 'T');
    EXPECT_TRUE(hat.IsWord());
    const auto& hat_children = hat.Children();
    EXPECT_EQ(hat_children.size(), 0);

    const auto& ho = h_children[1];
    EXPECT_EQ(ho.c(), 'O');
    EXPECT_FALSE(ho.IsWord());
    const auto& ho_children = ho.Children();
    EXPECT_EQ(ho_children.size(), 2);

    const auto& hor = ho_children[0];
    EXPECT_EQ(hor.c(), 'R');
    EXPECT_FALSE(hor.IsWord());
    const auto& hor_children = hor.Children();
    EXPECT_EQ(hor_children.size(), 1);

    const auto& hors = hor_children[0];
    EXPECT_EQ(hors.c(), 'S');
    EXPECT_FALSE(hors.IsWord());
    const auto& hors_children = hors.Children();
    EXPECT_EQ(hors_children.size(), 1);

    const auto& horse = hors_children[0];
    EXPECT_EQ(horse.c(), 'E');
    EXPECT_TRUE(horse.IsWord());
    const auto& horse_children = horse.Children();
    EXPECT_EQ(horse_children.size(), 0);

    const auto& z = root_children[2];
    EXPECT_EQ(z.c(), 'Z');
    EXPECT_FALSE(z.IsWord());
    const auto& z_children = z.Children();
    EXPECT_EQ(z_children.size(), 1);

    const auto& zz = z_children[0];
    EXPECT_EQ(zz.c(), 'Z');
    EXPECT_FALSE(zz.IsWord());
    const auto& zz_children = zz.Children();
    EXPECT_EQ(zz_children.size(), 1);

    const auto& zzz = zz_children[0];
    EXPECT_EQ(zzz.c(), 'Z');
    EXPECT_TRUE(zzz.IsWord());
    const auto& zzz_children = zzz.Children();
    EXPECT_EQ(zzz_children.size(), 0);
}
