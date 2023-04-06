#ifndef SRC_TRIE_DAWG_H
#define SRC_TRIE_DAWG_H

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/string_view.h"
#include "src/scrabble/strings.h"
#include "src/scrabble/tiles.h"

class Dawg {
 public:
  explicit Dawg(const Tiles& tiles) : tiles_(tiles) {}
  static std::unique_ptr<Dawg> CreateFromTextfile(const Tiles& tiles,
                                                  const std::string& filename);

  class Node {
   public:
    Node() : chars_(""), is_word_(false), index_(-1) {}
    explicit Node(std::string chars, int index)
        : chars_(chars), is_word_(false), index_(index) {}

    bool IsWord() const { return is_word_; }
    void SetIsWord(bool is_word) { is_word_ = is_word; }
    std::string Chars() const { return chars_; }
    void AddChars(const std::string& chars) { chars_ += chars; }

    const std::vector<int>& ChildIndices() const { return child_indices_; }
    std::vector<int>& ChildIndices() { return child_indices_; }
    void SetChildIndices(const std::vector<int>& child_indices) {
      child_indices_ = child_indices;
    }
    void AddChildIndex(int i) { child_indices_.emplace_back(i); }
    int Index() const { return index_; }
    void SetPosition(int position) { position_ = position; }
   private:
    std::string chars_;
    bool is_word_;
    std::vector<int> child_indices_;
    int index_;
    int position_;
  };

  const Node* Root() const { return &root_; }
  Node* Root() { return &root_; }

  const Node* NodeAt(int index) const { return &nodes_[index]; }
  Node* NodeAt(int index) { return &nodes_[index]; }

 private:
  FRIEND_TEST(DawgTest, CreateFromTextfile);
  FRIEND_TEST(DawgTest, MergeSingleChildrenIntoParents);
  FRIEND_TEST(DawgTest, MergeDuplicateSubtrees);
  FRIEND_TEST(DawgTest, CountNodeSubstrings);
  FRIEND_TEST(DawgTest, CountNodeSubstrings2);

  void PushWord(Node* node, absl::string_view word);
  void MergeSingleChildrenIntoParents();
  void MergeSingleChildrenIntoParents(Node* node);
  void MergeDuplicateSubtrees();
  void HashNodes(Node* node);
  bool NodesAreEqual(const Node* lhs, const Node*) const;
  void AddUniqueIndices(const Node* node,
                        absl::flat_hash_set<int>* unique_indices) const;
  void ConvertToBytes();
  int ConvertToBytes(Node* node, int pos);
  void CountNodeSubstrings(const Node* node);
  const Tiles& tiles_;
  absl::flat_hash_map<std::string, int> substring_counts_;
  Node root_;
  std::vector<Node> nodes_;
  std::map<size_t, std::vector<int>> hash_to_node_indices_;
  std::vector<uint8_t> bytes_;
};

#endif  // SRC_TRIE_DAWG_H