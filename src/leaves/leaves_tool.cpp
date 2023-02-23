#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/status/status.h"
#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/leaves/leaves.h"

ABSL_FLAG(std::string, input_csv, "", "input .csv file path");
ABSL_FLAG(std::string, output_file, "", "output .qlv file path");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  LOG(INFO) << "Hello world!" << std::endl;
  LOG(INFO) << "input_csw: " << absl::GetFlag(FLAGS_input_csv);
  LOG(INFO) << "output_file: " << absl::GetFlag(FLAGS_output_file);

  auto tiles = absl::make_unique<Tiles>(
        "src/leaves/testdata/english_scrabble_tiles.textproto");

  std::unique_ptr<Leaves> leaves = Leaves::CreateFromCsv(
      *tiles, absl::GetFlag(FLAGS_input_csv));
  if (!leaves->WriteToBinaryFile(absl::GetFlag(FLAGS_output_file)).ok()) {
    LOG(ERROR) << "Failed to write to file: " << absl::GetFlag(FLAGS_output_file);
    return 1;
  }
  return 0;
}