#include "src/trie/dawg.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
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
    LOG(INFO) << "Read word: " << word;
    dawg->PushWord(dawg->Root(), word);
  }
  return dawg;
}

void Dawg::PushWord(Node* node, absl::string_view word) {
  int current_node_index = node->Index();
  LOG(INFO) << "PushWord: node->Index(): " << node->Index()
            << ", word: " << word << ", node->Chars(): " << node->Chars()
            << ", node->IsWord(): " << node->IsWord();
  if (word.empty()) {
    LOG(INFO) << "PushWord: word is empty, setting node->IsWord(true)";
    node->SetIsWord(true);
    return;
  }
  const std::string first_char({word[0]});
  LOG(INFO) << "node->ChildIndices().size(): " << node->ChildIndices().size();
  // Copying child_indices from node because adding more nodes may invalidate
  // 'node'.
  const std::vector<int> child_indices = node->ChildIndices();
  for (int index : child_indices) {
    Node* child = NodeAt(index);
    LOG(INFO) << "child->Index(): " << child->Index()
              << ", child->Chars(): " << child->Chars();
    if (child->Chars() == first_char) {
      LOG(INFO) << "Found child with matching chars: " << child->Chars();
      // Note: his may invalidate 'node' by adding more nodes.
      PushWord(child, word.substr(1));
      return;
    } else {
      LOG(INFO) << child->Chars() << " != " << first_char;
    }
  }
  const int new_node_index = nodes_.size();
  nodes_.emplace_back(Node(first_char, new_node_index));
  if (current_node_index >= 0) {
    // Node may have changed address after adding a new node(s) to the
    // vector.
    node = &nodes_[current_node_index];
  }
  LOG(INFO) << "Adding new node with index: " << new_node_index;
  LOG(INFO) << "nodes_[new_node_index].Chars(): "
            << nodes_[new_node_index].Chars();
  node->AddChildIndex(new_node_index);
  LOG(INFO) << "child indices...";
  for (int index : node->ChildIndices()) {
    LOG(INFO) << "  index: " << index;
  }

  PushWord(&nodes_[new_node_index], word.substr(1));
}

void Dawg::MergeSingleChildrenIntoParents() {
  MergeSingleChildrenIntoParents(&root_);
}

void Dawg::MergeSingleChildrenIntoParents(Node* node) {
  LOG(INFO) << "MergeSingleChildrenIntoParents: " << node->Chars()
            << " children: " << node->ChildIndices().size();
  if (!node->IsWord() && node->ChildIndices().size() == 1) {
    Node* child = NodeAt(node->ChildIndices()[0]);
    node->SetIsWord(child->IsWord());
    node->AddChars(child->Chars());
    node->SetChildIndices(child->ChildIndices());
    MergeSingleChildrenIntoParents(node);
  }
  for (int index : node->ChildIndices()) {
    Node* child = NodeAt(index);
    MergeSingleChildrenIntoParents(child);
  }
}

namespace {
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
}  // namespace

bool Dawg::NodesAreEqual(const Node* lhs, const Node* rhs) const {
  if (lhs->ChildIndices().size() != rhs->ChildIndices().size()) {
    return false;
  }
  if (lhs->Chars() != rhs->Chars()) {
    return false;
  }
  if (lhs->IsWord() != rhs->IsWord()) {
    return false;
  }
  for (int i = 0; i < lhs->ChildIndices().size(); ++i) {
    if (!NodesAreEqual(NodeAt(lhs->ChildIndices()[i]),
                       NodeAt(rhs->ChildIndices()[i]))) {
      return false;
    }
  }
  return true;
}

void Dawg::MergeDuplicateSubtrees() {
  HashNodes(&root_);
  absl::flat_hash_map<int, int> replacement_indices;
  for (const auto& pair : hash_to_node_indices_) {
    const auto& indices = pair.second;
    if (indices.size() > 1) {
      LOG(INFO) << "Found " << indices.size()
                << " nodes with hash: " << pair.first;
      for (int i = 0; i < indices.size() - 1; ++i) {
        for (int j = i + 1; j < indices.size(); ++j) {
          if (replacement_indices.count(indices[j]) > 0) {
            continue;
          }
          if (NodesAreEqual(NodeAt(indices[i]), NodeAt(indices[j]))) {
            LOG(INFO) << "Found duplicate nodes: " << indices[i] << " and "
                      << indices[j];
            replacement_indices[indices[j]] = indices[i];
          }
        }
      }
    }
  }
  for (auto& node : nodes_) {
    for (int i = 0; i < node.ChildIndices().size(); ++i) {
      if (replacement_indices.count(node.ChildIndices()[i]) > 0) {
        node.ChildIndices()[i] = replacement_indices[node.ChildIndices()[i]];
      }
    }
  }
}

void Dawg::HashNodes(Node* node) {
  std::size_t hash = 0;
  hash_combine(hash, node->IsWord());
  hash_combine(hash, node->Chars());
  for (int index : node->ChildIndices()) {
    Node* child = NodeAt(index);
    hash_combine(hash, child->Index());
  }
  hash_to_node_indices_[hash].push_back(node->Index());
  for (int index : node->ChildIndices()) {
    Node* child = NodeAt(index);
    HashNodes(child);
  }
}

void Dawg::AddUniqueIndices(const Node* node, absl::flat_hash_set<int>* unique_indices) const {
  unique_indices->insert(node->Index());
  for (int index : node->ChildIndices()) {
    const Node* child = NodeAt(index);
    AddUniqueIndices(child, unique_indices);
  }
}