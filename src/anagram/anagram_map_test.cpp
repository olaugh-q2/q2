#include "src/anagram/anagram_map.h"

#include <range/v3/all.hpp>

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "src/anagram/anagram_map.pb.h"

using ::google::protobuf::Arena;
using ::google::protobuf::TextFormat;
using ::google::protobuf::util::MessageDifferencer;
using ::google::protobuf::util::ParseDelimitedFromZeroCopyStream;
using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

class AnagramMapTest : public testing::Test {
 protected:
  void SetUp() override {
    tiles_ = absl::make_unique<Tiles>(
        "src/anagram/testdata/english_scrabble_tiles.textproto");
  }

  Letter L(char c) { return tiles_->CharToNumber(c).value(); }
  LetterString LS(const std::string& s) {
    return tiles_->ToLetterString(s).value();
  }
  LetterPair LP(char c1, char c2) { return {L(c1), L(c2)}; }
  absl::uint128 P(const std::string& s) { return tiles_->ToProduct(LS(s)); }

  std::unique_ptr<Tiles> tiles_;
};

TEST_F(AnagramMapTest, CreateFromTextfile) {
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_THAT(
      anagram_map->map_,
      UnorderedElementsAre(Pair(P("AH"), ElementsAre(LS("AH"), LS("HA"))),
                           Pair(P("AHM"), ElementsAre(LS("HAM")))));

  EXPECT_THAT(anagram_map->blank_map_,
              UnorderedElementsAre(Pair(P("A"), ElementsAre(L('H'))),
                                   Pair(P("H"), ElementsAre(L('A'))),
                                   Pair(P("AH"), ElementsAre(L('M'))),
                                   Pair(P("AM"), ElementsAre(L('H'))),
                                   Pair(P("HM"), ElementsAre(L('A')))));

  EXPECT_THAT(anagram_map->double_blank_map_,
              UnorderedElementsAre(Pair(P(""), ElementsAre(LP('A', 'H'))),
                                   Pair(P("A"), ElementsAre(LP('H', 'M'))),
                                   Pair(P("H"), ElementsAre(LP('A', 'M'))),
                                   Pair(P("M"), ElementsAre(LP('A', 'H')))));
}

TEST_F(AnagramMapTest, CreateFromTextfile2) {
  const std::string input_filepath = "src/anagram/testdata/csw21nines.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_EQ(anagram_map->map_.size(), 39317);
  const auto words = anagram_map->Words(P("AEIOUCNRT"));
  ASSERT_NE(words, nullptr);
  EXPECT_THAT(*words, ElementsAre(LS("AUTOCRINE"), LS("CAUTIONER"),
                                  LS("COINTREAU"), LS("RECAUTION")));

  const auto blanks = anagram_map->Blanks(P("TAILERON"));
  EXPECT_THAT(*blanks, ElementsAre(L('A'),  // ALIENATOR, RATIONALE
                                   L('C'),  // CLARIONET, CROTALINE
                                   L('D'),  // RODENTIAL
                                   L('F'),  // REFLATION
                                   L('M'),  // MENTORIAL
                                   L('N'),  // INTERLOAN
                                   L('P'),  // PRELATION, RANTIPOLE
                                   L('S'),  // AEIOLNRST (x5)
                                   L('T'),  // NATROLITE, TENTORIAL
                                   L('U')   // OUTLINEAR
                                   ));

  const auto double_blanks = anagram_map->DoubleBlanks(P("STRONCK"));
  EXPECT_THAT(*double_blanks, ElementsAre(LP('A', 'L'),  // CORNSTALK
                                          LP('C', 'U'),  // TURNCOCKS
                                          LP('E', 'U'),  // COKERNUTS, ORESTUNCK
                                          LP('H', 'O')   // STOCKHORN
                                          ));

  const auto range = anagram_map->Words(P("ROOFLINE"), 1);
  EXPECT_EQ(range.Spans().size(), 2);
  EXPECT_EQ(range.Spans()[0].size(), 1);
  EXPECT_EQ(range.Spans()[1].size(), 2);
  auto span_join = range.Spans() | ranges::view::join;
  const auto range_words =
      std::vector<LetterString>(span_join.begin(), span_join.end());
  EXPECT_THAT(range_words,
              ElementsAre(LS("FOILBORNE"), LS("ROOFLINES"), LS("SOLFERINO")));
}

TEST_F(AnagramMapTest, WriteToOstream) {
  Arena arena;
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.txt";
  auto anagram_map = AnagramMap::CreateFromTextfile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);

  std::stringstream ss;
  EXPECT_TRUE(anagram_map->WriteToOstream(ss));
  LOG(INFO) << "ss.str(): " << ss.str();

  std::istringstream iss(ss.str());
  auto iis = std::make_unique<google::protobuf::io::IstreamInputStream>(&iss);

  auto words = Arena::CreateMessage<q2::proto::Words>(&arena);
  bool clean_eof = true;
  EXPECT_TRUE(ParseDelimitedFromZeroCopyStream(words, iis.get(), &clean_eof));
  EXPECT_FALSE(clean_eof);
  auto expected_words = Arena::CreateMessage<q2::proto::Words>(&arena);
  TextFormat::Parser parser;
  ASSERT_TRUE(parser.ParseFromString(R"pb(
                                       word: "AH" word: "HA" word: "HAM")pb",
                                     expected_words));
  EXPECT_TRUE(MessageDifferencer::Equivalent(*words, *expected_words));
  LOG(INFO) << "words: " << words->DebugString();
  LOG(INFO) << "expected_words: " << expected_words->DebugString();
  // EXPECT_TRUE(false);

  auto blank_letters = Arena::CreateMessage<q2::proto::BlankLetters>(&arena);
  EXPECT_TRUE(
      ParseDelimitedFromZeroCopyStream(blank_letters, iis.get(), &clean_eof));
  EXPECT_FALSE(clean_eof);
  LOG(INFO) << "blank_letters: " << blank_letters->DebugString();
  const std::string expected_blank_letters = {
      L('H'),  //    3 = A, A + H = AH/HA
      L('A'),  //   53 = H, H + A = AH/HA
      L('M'),  //  159 = 3*53 = AH, AH + M = HAM
      L('H'),  //  177 = 3*59 = AM, AM + H = HAM
      L('A')   // 3127 = 53*59 = HM, HM + A = HAM
  };
  EXPECT_EQ(blank_letters->letters(), expected_blank_letters);

  auto double_blank_letters =
      Arena::CreateMessage<q2::proto::DoubleBlankLetters>(&arena);
  EXPECT_TRUE(ParseDelimitedFromZeroCopyStream(double_blank_letters, iis.get(),
                                               &clean_eof));
  EXPECT_FALSE(clean_eof);
  LOG(INFO) << "double_blank_letters: " << double_blank_letters->DebugString();
  const std::string expected_double_blank_letter_pairs = {
      L('A'), L('H'),  //  1 = "", "" + AH = AH/HA
      L('H'), L('M'),  //  3 = A, A + HM = HAM
      L('A'), L('M'),  // 53 = H, H + AM = HAM
      L('A'), L('H')   // 59 = M, M + AH = HAM
  };
  EXPECT_EQ(double_blank_letters->letter_pairs(),
            expected_double_blank_letter_pairs);

  auto word_spans = Arena::CreateMessage<q2::proto::WordSpans>(&arena);
  EXPECT_TRUE(
      ParseDelimitedFromZeroCopyStream(word_spans, iis.get(), &clean_eof));
  EXPECT_FALSE(clean_eof);
  LOG(INFO) << "word_spans: " << word_spans->DebugString();
  auto expected_word_spans = Arena::CreateMessage<q2::proto::WordSpans>(&arena);
  ASSERT_TRUE(parser.ParseFromString(R"pb(
                                       word_spans {
                                         product { low: 159 }
                                         # begin: 0
                                         length: 2
                                       }
                                       word_spans {
                                         product { low: 9381 }
                                         begin: 2
                                         length: 1
                                       }
                                     )pb",
                                     expected_word_spans));
  EXPECT_TRUE(
      MessageDifferencer::Equivalent(*word_spans, *expected_word_spans));

  auto blank_spans = Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  EXPECT_TRUE(
      ParseDelimitedFromZeroCopyStream(blank_spans, iis.get(), &clean_eof));
  EXPECT_FALSE(clean_eof);
  LOG(INFO) << "blank_spans: " << blank_spans->DebugString();
  auto expected_blank_spans =
      Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  ASSERT_TRUE(parser.ParseFromString(R"pb(
                                       blank_spans {
                                         product { low: 3 }
                                         # begin: 0
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 53 }
                                         begin: 1
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 159 }
                                         begin: 2
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 177 }
                                         begin: 3
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 3127 }
                                         begin: 4
                                         length: 1
                                       }
                                     )pb",
                                     expected_blank_spans));
  EXPECT_TRUE(
      MessageDifferencer::Equivalent(*blank_spans, *expected_blank_spans));

  auto double_blank_spans = Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  EXPECT_TRUE(ParseDelimitedFromZeroCopyStream(double_blank_spans, iis.get(),
                                               &clean_eof));
  EXPECT_FALSE(clean_eof);
  LOG(INFO) << "double_blank_spans: " << double_blank_spans->DebugString();
  auto expected_double_blank_spans =
      Arena::CreateMessage<q2::proto::BlankSpans>(&arena);
  ASSERT_TRUE(parser.ParseFromString(R"pb(
                                       blank_spans {
                                         product { low: 1 }
                                         # begin: 0
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 3 }
                                         begin: 1
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 53 }
                                         begin: 2
                                         length: 1
                                       }
                                       blank_spans {
                                         product { low: 59 }
                                         begin: 3
                                         length: 1
                                       }
                                     )pb",
                                     expected_double_blank_spans));
  EXPECT_TRUE(MessageDifferencer::Equivalent(*double_blank_spans,
                                             *expected_double_blank_spans));
}

TEST_F(AnagramMapTest, CreateFromBinaryFile) {
  const std::string input_filepath = "src/anagram/testdata/ah_ha_ham.qam";
  auto anagram_map = AnagramMap::CreateFromBinaryFile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_THAT(
      anagram_map->map_,
      UnorderedElementsAre(Pair(P("AH"), ElementsAre(LS("AH"), LS("HA"))),
                           Pair(P("AHM"), ElementsAre(LS("HAM")))));

  EXPECT_THAT(anagram_map->blank_map_,
              UnorderedElementsAre(Pair(P("A"), ElementsAre(L('H'))),
                                   Pair(P("H"), ElementsAre(L('A'))),
                                   Pair(P("AH"), ElementsAre(L('M'))),
                                   Pair(P("AM"), ElementsAre(L('H'))),
                                   Pair(P("HM"), ElementsAre(L('A')))));

  EXPECT_THAT(anagram_map->double_blank_map_,
              UnorderedElementsAre(Pair(P(""), ElementsAre(LP('A', 'H'))),
                                   Pair(P("A"), ElementsAre(LP('H', 'M'))),
                                   Pair(P("H"), ElementsAre(LP('A', 'M'))),
                                   Pair(P("M"), ElementsAre(LP('A', 'H')))));
}

TEST_F(AnagramMapTest, CreateFromBinaryFile2) {
  const std::string input_filepath = "src/anagram/testdata/csw21.qam";
  auto anagram_map = AnagramMap::CreateFromBinaryFile(*tiles_, input_filepath);
  ASSERT_NE(anagram_map, nullptr);
  EXPECT_EQ(anagram_map->map_.size(), 247168);
  const auto words = anagram_map->Words(P("AEIOUCNRT"));
  ASSERT_NE(words, nullptr);
  EXPECT_THAT(*words, ElementsAre(LS("AUTOCRINE"), LS("CAUTIONER"),
                                  LS("COINTREAU"), LS("RECAUTION")));

  const auto blanks = anagram_map->Blanks(P("TAILERON"));
  EXPECT_THAT(*blanks, ElementsAre(L('A'),  // ALIENATOR, RATIONALE
                                   L('C'),  // CLARIONET, CROTALINE
                                   L('D'),  // RODENTIAL
                                   L('F'),  // REFLATION
                                   L('M'),  // MENTORIAL
                                   L('N'),  // INTERLOAN
                                   L('P'),  // PRELATION, RANTIPOLE
                                   L('S'),  // AEIOLNRST (x5)
                                   L('T'),  // NATROLITE, TENTORIAL
                                   L('U')   // OUTLINEAR
                                   ));

  const auto double_blanks = anagram_map->DoubleBlanks(P("STRONCK"));
  EXPECT_THAT(*double_blanks, ElementsAre(LP('A', 'L'),  // CORNSTALK
                                          LP('C', 'U'),  // TURNCOCKS
                                          LP('E', 'U'),  // COKERNUTS, ORESTUNCK
                                          LP('H', 'O')   // STOCKHORN
                                          ));

  const auto range = anagram_map->Words(P("ROOFLINE"), 1);
  EXPECT_EQ(range.Spans().size(), 2);
  EXPECT_EQ(range.Spans()[0].size(), 1);
  EXPECT_EQ(range.Spans()[1].size(), 2);
  auto span_join = range.Spans() | ranges::view::join;
  const auto range_words =
      std::vector<LetterString>(span_join.begin(), span_join.end());
  EXPECT_THAT(range_words,
              ElementsAre(LS("FOILBORNE"), LS("ROOFLINES"), LS("SOLFERINO")));

  const auto range2 = anagram_map->Words(P("INTERNATIONAL"), 2);
  auto span_join2 = range2.Spans() | ranges::view::join;
  const auto range_words2 =
      std::vector<LetterString>(span_join2.begin(), span_join2.end());
  EXPECT_THAT(range_words2,
              ElementsAre(LS("INTERLAMINATION"), LS("INTERNALISATION"),
                          LS("INTERNALIZATION"), LS("INTERNATIONALLY")));
}
