#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/str_join.h"
#include "magpie_rack.h"

ABSL_FLAG(std::string, input_wordlist, "", "input .txt file path");
ABSL_FLAG(std::string, output_word_sizes, "", "output .txt file path");

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  const MagpieRack english_bag = MagpieRackFromString(
      "AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLL"
      "MMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ??");

  const std::string input_wordlist = absl::GetFlag(FLAGS_input_wordlist);
  std::ifstream input(input_wordlist);
  std::string word;
  std::vector<std::string> words;
  while (std::getline(input, word)) {
    words.push_back(word);
  }

  const int rack_size = 7;
  const int max_playthrough = 5;
  const auto map =
      MakeRackWordSizesMap(words, english_bag, rack_size, max_playthrough);
  const std::string output_word_sizes = absl::GetFlag(FLAGS_output_word_sizes);
  std::ofstream output(output_word_sizes);
  for (const auto& pair : map) {
    const auto& key = pair.first;
    const auto& value = pair.second;
    const auto kth = std::next(value.begin(), max_playthrough + 1);
    const std::string value_string = absl::StrJoin(value.begin(), kth, " ");
    output << key << " " << value_string << std::endl;
  }
  return 0;
}