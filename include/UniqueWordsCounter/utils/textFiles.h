#pragma once

#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <string>
#include <unordered_set>
#include <vector>

extern const std::string kDataFolder;

extern const std::string kEmpty;
extern const std::string kSample;

extern const std::string kSyntheticShortWords10MB;
extern const std::string kSyntheticLongWords10MB;

extern const std::string kSyntheticShortWords100MB;
extern const std::string kSyntheticLongWords100MB;

extern const std::string kSyntheticShortWords1000MB;
extern const std::string kSyntheticLongWords1000MB;

extern const std::string kEnglishWords;

extern const std::initializer_list<std::string> kAllFiles;

auto getFile(const char *filename) -> std::ifstream;

auto getWords(std::initializer_list<std::string> filenames, bool shuffle = false)
    -> std::vector<std::string>;

auto getUniqueWords(std::initializer_list<std::string> filenames)
    -> std::unordered_set<std::string>;
