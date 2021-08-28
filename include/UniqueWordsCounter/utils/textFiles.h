#pragma once

#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <string>
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

auto getFile(const char *filename) -> std::ifstream;

auto getWords(std::initializer_list<std::string> filenames, bool shuffle = false)
    -> std::vector<std::string>;

auto getHashes(const std::vector<std::string> &words) -> std::vector<size_t>;
