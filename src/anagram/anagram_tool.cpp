#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/anagram/anagram_map.h"

ABSL_FLAG(std::string, input_textfile, "", "input .txt file path");
ABSL_FLAG(std::string, output_file, "", "output .qam file path");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  LOG(INFO) << "Hello world!" << std::endl;
  LOG(INFO) << "input_textfile: " << absl::GetFlag(FLAGS_input_textfile);
  LOG(INFO) << "output_file: " << absl::GetFlag(FLAGS_output_file);

  auto tiles = absl::make_unique<Tiles>(
        "src/anagram/testdata/english_scrabble_tiles.textproto");

  std::unique_ptr<AnagramMap> anagram_map = AnagramMap::CreateFromTextfile(
      *tiles, absl::GetFlag(FLAGS_input_textfile));
  if (!anagram_map->WriteToBinaryFile(absl::GetFlag(FLAGS_output_file)).ok()) {
    LOG(ERROR) << "Failed to write to file: " << absl::GetFlag(FLAGS_output_file);
    return 1;
  }
  return 0;
}