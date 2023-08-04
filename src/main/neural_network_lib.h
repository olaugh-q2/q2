#ifndef NEURAL_NETWORK_LIB_H
#define NEURAL_NETWORK_LIB_H

#include <vector>

class NeuralNetwork {
 public:
  NeuralNetwork(double learning_rate, int input_size, int hidden_size,
                int output_size);

  void Train(const std::vector<std::vector<double>>& inputs,
             const std::vector<std::vector<double>>& targets,
             int iterations);

  std::vector<double> Predict(const std::vector<double>& input) const;

 private:
  static double Sigmoid(double x);
  static double DSigmoid(double sigmoid_x);
  void InitializeWeights();
  double learning_rate_;
  int input_size_;
  int hidden_size_;
  int output_size_;
  std::vector<double> hidden_layer_;
  std::vector<double> output_layer_;
  std::vector<double> hidden_layer_bias_;
  std::vector<double> output_layer_bias_;

  std::vector<std::vector<double>> hidden_weights_;
  std::vector<std::vector<double>> output_weights_;
};

#endif  // NEURAL_NETWORK_LIB_H
