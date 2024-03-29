#include "src/trie/dawg.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
    dawg->PushWord(dawg->Root(), word);
  }
  return dawg;
}

void Dawg::PushWord(Node* node, absl::string_view word) {
  int current_node_index = node->Index();
  if (word.empty()) {
    node->SetIsWord(true);
    return;
  }
  const std::string first_char({word[0]});
  // Copying child_indices from node because adding more nodes may invalidate
  // 'node'.
  const std::vector<int> child_indices = node->ChildIndices();
  for (int index : child_indices) {
    Node* child = NodeAt(index);
    if (child->Chars() == first_char) {
      // Note: his may invalidate 'node' by adding more nodes.
      PushWord(child, word.substr(1));
      return;
    }
  }
  const int new_node_index = nodes_.size();
  nodes_.emplace_back(Node(first_char, new_node_index));
  if (current_node_index >= 0) {
    // Node may have changed address after adding a new node(s) to the
    // vector.
    node = &nodes_[current_node_index];
  }
  node->AddChildIndex(new_node_index);
  PushWord(&nodes_[new_node_index], word.substr(1));
}

void Dawg::MergeSingleChildrenIntoParents() {
  MergeSingleChildrenIntoParents(&root_);
}

void Dawg::MergeSingleChildrenIntoParents(Node* node) {
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
        if (replacement_indices.count(indices[i]) > 0) {
          //LOG(INFO) << "already merged " << indices[i] << " into "
          //          << replacement_indices[indices[i]];
          continue;
        }
        for (int j = i + 1; j < indices.size(); ++j) {
          if (NodesAreEqual(NodeAt(indices[i]), NodeAt(indices[j]))) {
            // LOG(INFO) << "Found duplicate nodes: " << indices[i] << " and "
            //           << indices[j];
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

void Dawg::AddUniqueIndices(const Node* node,
                            absl::flat_hash_set<int>* unique_indices) const {
  unique_indices->insert(node->Index());
  for (int index : node->ChildIndices()) {
    const Node* child = NodeAt(index);
    AddUniqueIndices(child, unique_indices);
  }
}

void Dawg::ConvertToBytes() { ConvertToBytes(&root_, 0); }

/*
int Dawg::ConvertToBytes(Node* node, int pos) {
  const int start_pos = pos;
  node->SetPosition(pos);
  CHECK_LT(node->Chars().size(), 128);
  std::bitset<8> contents(node->Chars().size());
  if (node->IsWord()) {
    contents.set(7);
  }
  bytes_[pos++] = contents.to_ulong();
  for (int i = 0; i < node->Chars().size(); ++i) {
    bytes_[pos++] = node->Chars()[i];
  }
  int end_pos = pos *= 3 * node->ChildIndices().size();
  for (int index : node->ChildIndices()) {
    Node* child = NodeAt(index);
    ConvertToBytes(child, pos);
    pos += child->Size();
  }
}
*/

void Dawg::CountNodeSubstrings(const Dawg::Node* node) {
  substring_counts_[node->Chars()]++;
  for (int index : node->ChildIndices()) {
    CountNodeSubstrings(Dawg::NodeAt(index));
  }
}