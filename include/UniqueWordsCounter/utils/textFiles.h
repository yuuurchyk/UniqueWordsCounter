#pragma once

#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

namespace UniqueWordsCounter::Utils::TextFiles
{
// TODO: refactor to use std::filesystem::path?
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

auto getFile(const std::string &filename) -> std::ifstream;

class WordsGenerator
{
public:
    WordsGenerator(std::initializer_list<std::string> files);

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

    std::vector<std::string> _files;

    std::vector<std::string>::const_iterator _currentFileName;
    std::ifstream                            _currentFile;

    std::string _word;
    size_t      _wordCounter{};
};

}    // namespace UniqueWordsCounter::Utils::TextFiles
