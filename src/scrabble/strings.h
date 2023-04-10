#ifndef SRC_SCRABBLE_STRINGS_H
#define SRC_SCRABBLE_STRINGS_H

#include <array>
#include <iterator>
#include <ostream>
#include <string_view>

#include "glog/logging.h"

#define FIXED_STRING_MAXIMUM_LENGTH 25

typedef char Letter;

class LetterString {
 public:
  typedef char* iterator;
  typedef const char* const_iterator;
  typedef unsigned int size_type;
  typedef char& reference;
  typedef const char& const_reference;

  LetterString();
  ~LetterString() {}
  LetterString(const Letter* s, size_t n);
  LetterString(size_t n, Letter c);
  LetterString(const Letter* s);
  LetterString(const LetterString& s);
  LetterString(LetterString&& s);

  int first_position() const;
  int last_position() const;
  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  void erase(const iterator i);
  std::size_t length() const;
  LetterString substr(std::size_t pos, std::size_t n) const;
  bool empty() const { return length() == 0; }
  std::size_t size() const;
  void clear() { first_position_ = 0; last_position_ = -1; }
  void push_back(Letter c);
  void pop_back();
  const Letter* constData() const { return data_; }

  int compare(const LetterString& s) const;

  LetterString& operator+=(Letter c);
  LetterString& operator+=(const LetterString& s);

  reference operator[](size_type i) { return data_[i]; }
  const_reference operator[](std::size_t i) const { return data_[i]; }
  LetterString& operator=(const LetterString& s);
  LetterString& operator=(LetterString&& s);

  std::array<int, 32> Counts() const {
    std::array<int, 32> ret;
    ret.fill(0);
    for (auto i = first_position(); i <= last_position(); i++) {
      ret[data_[i]]++;
    }
    return ret;
  }

 private:
  friend void PrintTo(const LetterString& bar, std::ostream* os) {
    for (size_t i = bar.first_position(); i <= bar.last_position(); i++) {
      char c = bar[i] + 'A' - 1;
      if (bar[i] == 27) {
        c = '?';
      }
      os->put(c);
    }
  }

  Letter data_[FIXED_STRING_MAXIMUM_LENGTH];
  int first_position_;
  int last_position_;
};

inline int LetterString::compare(const LetterString& s) const {
  int size1 = size();
  int size2 = s.size();
  int sz = (size1 < size2) ? size1 : size2;
  for (int i = 0; i < sz; ++i) {
    if (data_[i] < s.data_[i]) {
      return -1;
    } else if (data_[i] > s.data_[i]) {
      return 1;
    }
  }
  if (size1 > size2) {
    return 1;
  } else if (size2 > size1) {
    return -1;
  }
  return 0;
}

inline std::size_t LetterString::length() const {
  return last_position_ - first_position_ + 1;
}

inline std::size_t LetterString::size() const {
  return last_position_ - first_position_ + 1;
}

inline LetterString::const_iterator LetterString::begin() const {
  return data_ + first_position();
}

inline LetterString::const_iterator LetterString::end() const {
  return data_ + last_position() + 1;
}

inline LetterString::iterator LetterString::begin() {
  return data_ + first_position();
}

inline LetterString::iterator LetterString::end() {
  return data_ + last_position() + 1;
}

inline bool operator<(const LetterString& lhs, const LetterString& rhs) {
  return lhs.compare(rhs) < 0;
}

inline bool operator==(const LetterString& lhs, const LetterString& rhs) {
  return lhs.compare(rhs) == 0;
}

inline bool operator!=(const LetterString& lhs, const LetterString& rhs) {
  return lhs.compare(rhs) != 0;
}

template <typename H>
H AbslHashValue(H h, const LetterString& m) {
  const std::string_view s(&m.constData()[m.first_position()], m.size());
  h = H::combine(std::move(h), s);
  /*
  for (auto letter : m) {
    h = H::combine(std::move(h), letter);
  }
  */
  return h;
}

inline LetterString::LetterString() : first_position_(0), last_position_(-1) {}

inline LetterString::LetterString(const char* s, std::size_t n)
    : first_position_(0), last_position_(n - 1) {
  if (n > FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
    return;
  }
  for (size_t i = 0; i < n; i++) {
    data_[i] = s[i];
  }
}

inline LetterString::LetterString(size_t n, Letter c) {
  if (n > FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
    return;
  }
  for (size_t i = 0; i < n; i++) {
    data_[i] = c;
  }
  first_position_ = 0;
  last_position_ = n - 1;
}

inline LetterString::LetterString(const char* s) {
  size_t sz = strlen(s);
  if (sz > FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
  }
  memcpy(data_, s, sz);
  last_position_ = sz - 1;
}

inline LetterString::LetterString(const LetterString& s) {
  // LOG(INFO) << "called copy constructor";
  int sz = s.size();
  memcpy(data_ + s.first_position(), s.data_ + s.first_position(), sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
}

inline LetterString::LetterString(LetterString&& s) {
  // LOG(INFO) << "called std::move constructor";
  int sz = s.size();
  memcpy(data_ + s.first_position(), s.data_ + s.first_position(), sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
}

inline void LetterString::push_back(char c) {
  if (last_position_ + 1 >= FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
    return;
  }
  data_[++last_position_] = c;
}

inline int LetterString::first_position() const { return first_position_; }
inline int LetterString::last_position() const { return last_position_; }

inline LetterString& LetterString::operator=(const LetterString& s) {
  // LOG(INFO) << "called operator=";
  int sz = s.size();
  memcpy(data_, s.data_, sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
  return *this;
}

inline LetterString& LetterString::operator=(LetterString&& s) {
  // LOG(INFO) << "called std::move operator=";
  int sz = s.size();
  memcpy(data_, s.data_, sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
  return *this;
}

#endif  // SRC_SCRABBLE_STRINGS_H