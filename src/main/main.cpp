#include <iostream>
#include <iterator>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "src/anagram/anagram_map.h"

std::ostream &operator<<(std::ostream &dest, __int128_t value) {
  std::ostream::sentry s(dest);
  if (s) {
    __uint128_t tmp = value < 0 ? -value : value;
    char buffer[128];
    char *d = std::end(buffer);
    do {
      --d;
      *d = "0123456789"[tmp % 10];
      tmp /= 10;
    } while (tmp != 0);
    if (value < 0) {
      --d;
      *d = '-';
    }
    int len = std::end(buffer) - d;
    if (dest.rdbuf()->sputn(d, len) != len) {
      dest.setstate(std::ios_base::badbit);
    }
  }
  return dest;
}

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  //FLAGS_alsologtostderr = true;
  //google::InitGoogleLogging(argv[0]);
  //LOG(INFO) << "Hello world!" << std::endl;
  __int128_t product = 1;
  for (int i = 0; i < 70; i++) {
    product *= 3;
  }
  //LOG(INFO) << "product: " << product << std::endl;

  std::unique_ptr<AnagramMap> anagram_map = AnagramMap::CreateFromTextfile(
      "/Users/john/sources/bazel-q2/q2/src/main/words.txt");
  return 0;
}