#include "src/scrabble/strings.h"

#include <iterator>

#include "glog/logging.h"

LetterString::LetterString() : first_position_(0), last_position_(-1) {}

LetterString::LetterString(const char* s, std::size_t n)
    : first_position_(0), last_position_(n - 1) {
  if (n > FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
    return;
  }
  for (size_t i = 0; i < n; i++) {
    data_[i] = s[i];
  }
}

LetterString::LetterString(size_t n, Letter c) {
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

LetterString::LetterString(const char* s) {
  size_t sz = strlen(s);
  if (sz > FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
  }
  memcpy(data_, s, sz);
  last_position_ = sz - 1;
}

LetterString::LetterString(const LetterString& s) {
  int sz = s.size();
  memcpy(data_ + s.first_position(), s.data_ + s.first_position(), sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
}

LetterString::LetterString(LetterString&& s) {
  int sz = s.size();
  memcpy(data_ + s.first_position(), s.data_ + s.first_position(), sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
}

std::size_t LetterString::length() const {
  return last_position_ - first_position_ + 1;
}

void LetterString::push_back(char c) {
  if (last_position_ + 1 >= FIXED_STRING_MAXIMUM_LENGTH) {
    LOG(FATAL) << "String is too long";
    return;
  }
  data_[++last_position_] = c;
}

std::size_t LetterString::first_position() const { return first_position_; }
std::size_t LetterString::last_position() const { return last_position_; }

inline LetterString& LetterString::operator=(const LetterString& s) {
  int sz = s.size();
  memcpy(data_, s.data_, sz);
  first_position_ = s.first_position();
  last_position_ = s.last_position();
  return *this;
}
