#include "src/main/neural_network_lib.h"

#include <math.h>

#include <algorithm>
#include <random>

#include "glog/logging.h"

NeuralNetwork::NeuralNetwork(double learning_rate, int input_size,
                             int hidden_size, int output_size)
    : learning_rate_(learning_rate),
      input_size_(input_size),
      hidden_size_(hidden_size),
      output_size_(output_size) {
  hidden_layer_.resize(hidden_size_);
  output_layer_.resize(output_size_);
  hidden_layer_bias_.resize(hidden_size_);
  output_layer_bias_.resize(output_size_);

  hidden_weights_.resize(input_size_);
  for (int i = 0; i < input_size_; i++) {
    hidden_weights_[i].resize(hidden_size_);
  }

  output_weights_.resize(hidden_size_);
  for (int i = 0; i < hidden_size_; i++) {
    output_weights_[i].resize(output_size_);
  }
  InitializeWeights();
}

void NeuralNetwork::InitializeWeights() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> d(0, 1);

  for (int i = 0; i < input_size_; i++) {
    for (int j = 0; j < hidden_size_; j++) {
      hidden_weights_[i][j] = d(gen) / sqrt(input_size_);
    }
  }

  for (int i = 0; i < hidden_size_; i++) {
    for (int j = 0; j < output_size_; j++) {
      output_weights_[i][j] = d(gen) / sqrt(hidden_size_);
    }
  }

  for (int i = 0; i < hidden_size_; i++) {
    hidden_layer_bias_[i] = d(gen) / sqrt(hidden_size_);
  }

  for (int i = 0; i < output_size_; i++) {
    output_layer_bias_[i] = d(gen) / sqrt(output_size_);
  }
}

double NeuralNetwork::Sigmoid(double x) { return 1 / (1 + exp(-x)); }

double NeuralNetwork::DSigmoid(double sigmoid_x) {
  return sigmoid_x * (1 - sigmoid_x);
}

void NeuralNetwork::Train(const std::vector<std::vector<double>>& inputs,
                          const std::vector<std::vector<double>>& targets,
                          int iterations) {
  std::vector<int> shuffled_indices(inputs.size());
  for (int i = 0; i < shuffled_indices.size(); i++) {
    shuffled_indices[i] = i;
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  for (int iteration = 0; iteration < iterations; ++iteration) {
    //LOG(INFO) << "Iteration#: " << iteration;
    std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), gen);

    for (int i = 0; i < inputs.size(); i++) {
      const int shuffled_index = shuffled_indices[i];

      // Hidden layer activations
      for (int j = 0; j < hidden_size_; j++) {
        double sum = 0;
        for (int k = 0; k < input_size_; k++) {
          sum += inputs[shuffled_index][k] * hidden_weights_[k][j];
        }
        hidden_layer_[j] = Sigmoid(sum + hidden_layer_bias_[j]);
      }

      // Output layer activations
      for (int j = 0; j < output_size_; ++j) {
        double sum = 0;
        for (int k = 0; k < hidden_size_; ++k) {
          sum += hidden_layer_[k] * output_weights_[k][j];
        }
        output_layer_[j] = Sigmoid(sum + output_layer_bias_[j]);
      }

/*
      LOG(INFO) << "Iteration#: " << iteration;
      LOG(INFO) << "Input: " << inputs[shuffled_index][0] << " "
                << inputs[shuffled_index][1];
      LOG(INFO) << "Target: " << targets[shuffled_index][0];
      LOG(INFO) << "Predicted output: " << output_layer_[0];
*/
      // Back propagation
      std::vector<double> output_delta(output_size_);
      for (int j = 0; j < output_size_; ++j) {
        output_delta[j] = (targets[shuffled_index][j] - output_layer_[j]) *
                          DSigmoid(output_layer_[j]);
      }
      std::vector<double> hidden_delta(hidden_size_);
      for (int j = 0; j < hidden_size_; ++j) {
        double sum = 0;
        for (int k = 0; k < output_size_; ++k) {
          sum += output_delta[k] * output_weights_[j][k];
        }
        hidden_delta[j] = sum * DSigmoid(hidden_layer_[j]);
      }

      // Update weights
      for (int j = 0; j < output_size_; ++j) {
        output_layer_bias_[j] += learning_rate_ * output_delta[j];
        for (int k = 0; k < hidden_size_; ++k) {
          output_weights_[k][j] +=
              learning_rate_ * output_delta[j] * hidden_layer_[k];
        }
      }

      for (int j = 0; j < hidden_size_; ++j) {
        hidden_layer_bias_[j] += learning_rate_ * hidden_delta[j];
        for (int k = 0; k < input_size_; ++k) {
          hidden_weights_[k][j] +=
              learning_rate_ * hidden_delta[j] * inputs[shuffled_index][k];
        }
      }
/*
      LOG(INFO) << "Updated weights:";
      for (int j = 0; j < hidden_size_; ++j) {
        for (int k = 0; k < input_size_; ++k) {
          LOG(INFO) << "hidden_weights_[" << k << "][" << j
                    << "]: " << hidden_weights_[k][j];
        }
      }
      for (int j = 0; j < output_size_; ++j) {
        for (int k = 0; k < hidden_size_; ++k) {
          LOG(INFO) << "output_weights_[" << j << "][" << k
                    << "]: " << output_weights_[j][k];
        }
      }
      */
    }
  }
}

std::vector<double> NeuralNetwork::Predict(
    const std::vector<double>& input) const {
  std::vector<double> hidden_layer(hidden_size_);
  std::vector<double> output_layer(output_size_);

  // Hidden layer activations
  for (int j = 0; j < hidden_size_; j++) {
    double sum = 0;
    for (int k = 0; k < input_size_; k++) {
      sum += input[k] * hidden_weights_[k][j];
    }
    hidden_layer[j] = Sigmoid(sum + hidden_layer_bias_[j]);
  }

  // Output layer activations
  for (int j = 0; j < output_size_; ++j) {
    double sum = 0;
    for (int k = 0; k < hidden_size_; ++k) {
      sum += hidden_layer[k] * output_weights_[k][j];
    }
    output_layer[j] = Sigmoid(sum + output_layer_bias_[j]);
  }

  return output_layer;
}
