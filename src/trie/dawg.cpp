#include "src/trie/dawg.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "glog/logging.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

std::unique_ptr<Dawg> Dawg::CreateFromTextfile(const Tiles& tiles,
                                               const std::string& filename) {
  auto dawg = absl::make_unique<Dawg>(tiles);
  std::ifstream input(filename);
  if (!input) {
    LOG(ERROR) << "Failed to open " << filename;
    return nullptr;
  }
  std::string word;
  std::set<std::string> words;
  while (std::getline(input, word)) {
    if (word.empty()) {
      continue;
    }
    dawg->root_.PushWord(word);
  }
  return dawg;
}

void Dawg::Node::PushWord(absl::string_view word) {
  if (word.empty()) {
    is_word_ = true;
    return;
  }
  for (auto& child : children_) {
    if (child.c_ == word[0]) {
      child.PushWord(word.substr(1));
      return;
    }
  }
  children_.push_back(Node(word[0]));
  children_.back().PushWord(word.substr(1));
}