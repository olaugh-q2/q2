#ifndef SRC_TRIE_DAWG_H_
#define SRC_TRIE_DAWG_H_

#include "absl/container/flat_hash_map.h"
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
    Node() : c_(0), is_word_(false) {}
    void PushWord(absl::string_view word);
    bool IsWord() const { return is_word_; }
    char c() const { return c_; }
    const std::vector<Node>& Children() const { return children_; }

   private:
    explicit Node(char c) : c_(c), is_word_(false) {}
    const char c_;
    bool is_word_;
    std::vector<Node> children_;
  };

  const Node* Root() const { return &root_; }

 private:
  const Tiles& tiles_;
  absl::flat_hash_map<std::string, int> substring_counts_;
  Node root_;
};

#endif  // SRC_TRIE_DAWG_H_