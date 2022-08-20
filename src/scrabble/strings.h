#ifndef SRC_SCRABBLE_STRINGS_H_
#define SRC_SCRABBLE_STRINGS_H_

#include <iterator>
#include <ostream>

#define FIXED_STRING_MAXIMUM_LENGTH 25

class LetterString {
 public:
  typedef char* iterator;
  typedef const char* const_iterator;
  typedef unsigned int size_type;
  typedef char& reference;
  typedef const char& const_reference;

  LetterString();
  LetterString(const char* s, size_t n);
  LetterString(size_t n, char c);
  LetterString(const char* s);
  LetterString(const LetterString& s);
  LetterString(LetterString&& s);

  std::size_t first_position() const;
  std::size_t last_position() const;
  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  void erase(const iterator i);
  std::size_t length() const;
  LetterString substr(std::size_t pos, std::size_t n) const;
  bool empty() const;
  std::size_t size() const { return length(); }
  void clear();
  void push_back(char c);
  void pop_back();
  const char* constData() const { return data_; }

  int compare(const LetterString& s) const;

  LetterString& operator+=(char c);
  LetterString& operator+=(const LetterString& s);

  const_reference operator[](std::size_t i) const { return data_[i]; }
  LetterString& operator=(const LetterString& s);

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

  char data_[FIXED_STRING_MAXIMUM_LENGTH];
  std::size_t first_position_;
  std::size_t last_position_;
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

class DesignatedString {
 private:
  LetterString tiles_;
  LetterString letters_;
};

#endif  // SRC_SCRABBLE_STRINGS_H_