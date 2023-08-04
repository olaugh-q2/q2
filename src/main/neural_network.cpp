#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "glog/logging.h"
#include "src/main/neural_network_lib.h"

ABSL_FLAG(std::string, train_csv, "", "input training .csv file path");
ABSL_FLAG(std::string, test_csv, "", "input testing .csv file path");

std::string ToAsciiArt(const std::vector<double>& pixels) {
  std::string ascii_chars = " .:-=+*#%@";
  std::string ascii_art;

  for (const auto pixel : pixels) {
    int index = static_cast<int>(pixel * (ascii_chars.size() - 1) + 0.5);
    ascii_art += ascii_chars[index];
  }

  return ascii_art;
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  LOG(INFO) << "train_csv: " << absl::GetFlag(FLAGS_train_csv);
  LOG(INFO) << "test_csv: " << absl::GetFlag(FLAGS_test_csv);
  NeuralNetwork nn(0.05, 28 * 28, 64, 10);

  std::ifstream train_csv(absl::GetFlag(FLAGS_train_csv));
  if (!train_csv) {
    LOG(ERROR) << "Failed to open file: " << absl::GetFlag(FLAGS_train_csv);
    return 1;
  }

  std::ifstream test_csv(absl::GetFlag(FLAGS_test_csv));
  if (!test_csv) {
    LOG(ERROR) << "Failed to open file: " << absl::GetFlag(FLAGS_test_csv);
    return 1;
  }

  std::vector<std::vector<double>> train_inputs;
  std::vector<std::vector<double>> train_outputs;
  std::string line;
  while (std::getline(train_csv, line)) {
    std::vector<double> input(28 * 28);
    std::vector<double> output(10);
    std::istringstream iss(line);
    std::string token;
    const std::vector<std::string> tokens = absl::StrSplit(line, ',');
    for (int i = 0; i < 28 * 28; i++) {
      int value;
      auto result = absl::SimpleAtoi(tokens[i + 1], &value);
      CHECK(result) << "Failed to parse: " << tokens[i + 1];
      input[i] = value / 255.0;
    }
    for (int i = 0; i < 10; i++) {
      int value;
      auto result = absl::SimpleAtoi(tokens[0], &value);
      CHECK(result) << "Failed to parse: " << tokens[0];
      output[i] = value == i ? 1 : 0;
    }
    train_inputs.push_back(input);
    train_outputs.push_back(output);
  }
  LOG(INFO) << "Loaded " << train_inputs.size() << " training examples.";

  std::vector<std::vector<double>> test_inputs;
  std::vector<std::vector<double>> test_outputs;
  while (std::getline(test_csv, line)) {
    std::vector<double> input(28 * 28);
    std::vector<double> output(10);
    std::istringstream iss(line);
    std::string token;
    const std::vector<std::string> tokens = absl::StrSplit(line, ',');
    for (int i = 0; i < 28 * 28; i++) {
      int value;
      auto result = absl::SimpleAtoi(tokens[i + 1], &value);
      CHECK(result) << "Failed to parse: " << tokens[i + 1];
      input[i] = value / 255.0;
    }
    for (int i = 0; i < 10; i++) {
      int value;
      auto result = absl::SimpleAtoi(tokens[0], &value);
      CHECK(result) << "Failed to parse: " << tokens[0];
      output[i] = value == i ? 1 : 0;
    }
    test_inputs.push_back(input);
    test_outputs.push_back(output);
  }
  LOG(INFO) << "Loaded " << test_inputs.size() << " testing examples.";

  for (int epochs = 0; epochs < 100; epochs++) {
    nn.Train(train_inputs, train_outputs, 1);
    int correct = 0;
    for (int i = 0; i < test_inputs.size(); i++) {
      /*
    for (int j = 0; j < 28; j++) {
      LOG(INFO) << ToAsciiArt(
          std::vector<double>(test_inputs[i].begin() + j * 28,
                              test_inputs[i].begin() + (j + 1) * 28));
    }
    */
      const std::vector<double> prediction = nn.Predict(test_inputs[i]);
      int prediction_index = 0;
      for (int j = 0; j < prediction.size(); j++) {
        if (prediction[j] > prediction[prediction_index]) {
          prediction_index = j;
        }
      }
      int output_index = 0;
      for (int j = 0; j < test_outputs[i].size(); j++) {
        if (test_outputs[i][j] > test_outputs[i][output_index]) {
          output_index = j;
        }
      }
      if (prediction_index == output_index) {
        correct++;
      }
    }
    LOG(INFO) << "Epoch " << epochs << ": " << correct << " / "
              << test_inputs.size() << " = "
              << correct * 100.0 / test_inputs.size() << "%";
  }
  return 0;
}