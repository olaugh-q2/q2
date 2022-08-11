#include "anagram_map.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "glog/logging.h"

std::unique_ptr<AnagramMap> AnagramMap::CreateFromTextfile(const std::string& filename) {
    auto anagram_map = absl::make_unique<AnagramMap>();
    std::ifstream input(filename);
    if (!input) {
        LOG(ERROR) << "Failed to open " << filename;
        return nullptr;
    }
    std::string line;
    int i = 0;
    while (std::getline(input, line))
    {
        std::stringstream ss(line);
        std::string word;
        LOG(INFO) << "word: " << line;
        while (std::getline(ss, word, ' '))
        {
            i++;
            if (i % 10000 == 0)
            {
                std::cout << "i: " << i << " (" << word << ")" << std::endl;
            }
            }
    }
    return anagram_map;
}
