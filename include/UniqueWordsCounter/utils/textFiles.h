#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

namespace UniqueWordsCounter::Utils::TextFiles
{
extern const std::filesystem::path kEmpty;
extern const std::filesystem::path kSample;

extern const std::filesystem::path kSyntheticShortWords10MB;
extern const std::filesystem::path kSyntheticLongWords10MB;

extern const std::filesystem::path kSyntheticShortWords100MB;
extern const std::filesystem::path kSyntheticLongWords100MB;

extern const std::filesystem::path kSyntheticShortWords1000MB;
extern const std::filesystem::path kSyntheticLongWords1000MB;

extern const std::filesystem::path kEnglishWords;

extern const std::initializer_list<std::filesystem::path> kAllFiles;

auto getFile(const std::filesystem::path &filename) -> std::ifstream;

class WordsGenerator
{
public:
    WordsGenerator(std::initializer_list<std::filesystem::path> files);

    WordsGenerator(const WordsGenerator &) = delete;
    WordsGenerator &operator=(const WordsGenerator &) = delete;

    WordsGenerator(WordsGenerator &&) = default;
    WordsGenerator &operator=(WordsGenerator &&) = default;

    ~WordsGenerator() = default;

    class iterator
    {
    public:
        explicit iterator(WordsGenerator &);
        explicit iterator(WordsGenerator &, std::nullptr_t);

        using iterator_category = std::input_iterator_tag;
        using value_type        = const std::string;
        using pointer           = const std::string *;
        using reference         = const std::string &;

        inline reference operator*() const noexcept { return _word; }
        inline pointer   operator->() const noexcept { return &_word; }

        bool operator==(const iterator &) const;
        bool operator!=(const iterator &rhs) const { return !((*this) == rhs); }

        iterator &operator++();
        iterator  operator++(int);

    private:
        WordsGenerator &_instance;

        std::string _word;
        size_t      _wordCounter{};
    };

    iterator begin() { return iterator{ *this }; }
    iterator end() { return iterator{ *this, nullptr }; }

private:
    void advance();

    std::vector<std::filesystem::path> _files;

    std::vector<std::filesystem::path>::const_iterator _currentFileName;
    std::ifstream                                      _currentFile;

    std::string _word;
    size_t      _wordCounter{};
};

}    // namespace UniqueWordsCounter::Utils::TextFiles
