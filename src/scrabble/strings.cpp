#include "strings.h"

#include "glog/logging.h"

LetterString::LetterString() : first_position_(0),
                               last_position_(0) {}

LetterString::LetterString(const char *s, size_type n) : first_position_(0),
                                                         last_position_(n - 1)
{
    if (n > FIXED_STRING_MAXIMUM_LENGTH)
    {
        LOG(FATAL) << "String is too long";
        return;
    }
}
