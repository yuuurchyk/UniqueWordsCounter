#include "UniqueWordsCounter/utils/textFiles.h"

#include <filesystem>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
void checkFilenameValid(const char *filename)
{
    if (filename == nullptr)
        throw std::runtime_error{ "Filename cannot be nullptr" };

    if (!std::filesystem::is_regular_file(filename))
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "File " << filename << " does not point to a regular file";

        throw std::runtime_error{ errorMessage.str() };
    }
}

}    // namespace

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
    checkFilenameValid(filename);

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

WordsGenerator::WordsGenerator(std::initializer_list<std::string> files)
    : _files{ files.begin(), files.end() }, _currentFileName{ _files.begin() }
{
    for (const auto &filename : _files)
        checkFilenameValid(filename.c_str());

    if (!_files.empty())
        _currentFile = getFile(_currentFileName->c_str());

    advance();
}

void WordsGenerator::advance()
{
    if (_currentFileName == _files.end())
        return _word.clear();

    if (_currentFile >> _word)
    {
        ++_wordCounter;
        return;
    }
    else
    {
        ++_currentFileName;
        return advance();
    }
}

WordsGenerator::iterator::iterator(WordsGenerator *instance) : _instance{ instance }
{
    if (_instance != nullptr)
    {
        _word        = std::move(instance->_word);
        _wordCounter = instance->_wordCounter;
    }
}

bool WordsGenerator::iterator::operator==(const iterator &rhs) const
{
    if (_word.empty() && rhs._word.empty())
        return true;

    return _wordCounter == rhs._wordCounter;
}

WordsGenerator::iterator &WordsGenerator::iterator::operator++()
{
    if (_instance != nullptr)
    {
        _instance->advance();

        _word        = std::move(_instance->_word);
        _wordCounter = _instance->_wordCounter;
    }

    return *this;
}

WordsGenerator::iterator WordsGenerator::iterator::operator++(int)
{
    auto self = (*this);
    ++(*this);
    return self;
}

WordsGenerator::iterator WordsGenerator::begin()
{
    return iterator{ this };
}

WordsGenerator::iterator WordsGenerator::end()
{
    return iterator{};
}
