#include "absl/flags/flag.h"
#include "gtest/gtest.h"
#include "src/main/neural_network_lib.h"

TEST(NeuralNetworkTest, Xor) {
  NeuralNetwork nn(0.2, 2, 10, 1);
  std::vector<std::vector<double>> inputs = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
  std::vector<std::vector<double>> outputs = {{0}, {1}, {1}, {0}};
  nn.Train(inputs, outputs, 3000);
  for (int i = 0; i < inputs.size(); i++) {
    const std::vector<double> prediction = nn.Predict(inputs[i]);
    EXPECT_NEAR(prediction[0], outputs[i][0], 0.1);
  }
}