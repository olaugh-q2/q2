#define FIXED_STRING_MAXIMUM_LENGTH 25

class LetterString {
  public:
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef unsigned int size_type;
    typedef char& reference;
    typedef const char& const_reference;

    LetterString();
    LetterString(const char* s, size_type n);
    LetterString(size_type n, char c);
    LetterString(const char* s);
    LetterString(const LetterString& s);
    LetterString(LetterString&& s);

    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();
    void erase(const iterator i);
    size_type length() const;
    LetterString substr(size_type pos, size_type n) const;
    bool empty() const;
    size_type size() const { return length(); }
    void clear();
    void push_back(char c);
    void pop_back();
    const char* constData() const { return data_; }

    int compare(const LetterString& s) const;

    LetterString& operator+=(char c);
    LetterString& operator+=(const LetterString& s);

    const_reference operator[](size_type i) const { return data_[i]; }
    LetterString& operator=(const LetterString &s);

  private:
    char data_[FIXED_STRING_MAXIMUM_LENGTH];
    int first_position_;
    int last_position_;
};

class DesignatedString {
  private:
    LetterString tiles_;
    LetterString letters_;
};