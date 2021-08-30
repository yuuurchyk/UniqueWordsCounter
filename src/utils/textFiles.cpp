#include "UniqueWordsCounter/utils/textFiles.h"

#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

const std::string kDataFolder{ DATA_FOLDER };

const std::string kEmpty{ kDataFolder + "/empty.txt" };
const std::string kSample{ kDataFolder + "/sample.txt" };

const std::string kSyntheticShortWords10MB{ kDataFolder +
                                            "/syntheticShortWords10MB.txt" };
const std::string kSyntheticLongWords10MB{ kDataFolder + "/syntheticLongWords10MB.txt" };

const std::string kSyntheticShortWords100MB{ kDataFolder +
                                             "/syntheticShortWords100MB.txt" };
const std::string kSyntheticLongWords100MB{ kDataFolder +
                                            "/syntheticLongWords100MB.txt" };

const std::string kSyntheticShortWords1000MB{ kDataFolder +
                                              "/syntheticShortWords1000MB.txt" };
const std::string kSyntheticLongWords1000MB{ kDataFolder +
                                             "/syntheticLongWords1000MB.txt" };

const std::string kEnglishWords{ kDataFolder + "/englishWords.txt" };

const std::initializer_list<std::string> kAllFiles{ kEmpty,
                                                    kSample,
                                                    kSyntheticShortWords10MB,
                                                    kSyntheticLongWords10MB,
                                                    kSyntheticShortWords100MB,
                                                    kSyntheticLongWords100MB,
                                                    kSyntheticShortWords1000MB,
                                                    kSyntheticLongWords1000MB };

auto getFile(const char *filename) -> std::ifstream
{
    if (filename == nullptr)
        throw std::runtime_error{ "Filename cannot be nullptr" };

    auto file = std::ifstream{ filename };

    if (!file.is_open())
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Cannot open file: " << filename;
        throw std::runtime_error{ errorMessage.str() };
    }

    return file;
}

auto getWords(std::initializer_list<std::string> filenames, bool shuffle)
    -> std::vector<std::string>
{
    auto words = std::vector<std::string>{};

    for (const auto &filename : filenames)
    {
        auto file = getFile(filename.c_str());
        auto word = std::string{};

        while (file >> word)
            words.push_back(std::move(word));
    }

    if (shuffle)
        std::shuffle(words.begin(), words.end(), std::mt19937{ 47 });

    return words;
}

auto getUniqueWords(std::initializer_list<std::string> filenames)
    -> std::unordered_set<std::string>
{
    auto uniqueWords = std::unordered_set<std::string>{};

    for (const auto &filename : filenames)
    {
        auto file = getFile(filename.c_str());
        auto word = std::string{};

        while (file >> word)
            uniqueWords.insert(std::move(word));
    }

    return uniqueWords;
}
