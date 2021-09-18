#include "UniqueWordsCounter/utils/textFiles.h"

#include <filesystem>
#include <random>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

namespace
{
auto checkFilenameValid(const fs::path &filename) -> void
{
    using namespace std::string_literals;
    if (!fs::is_regular_file(filename))
        throw std::runtime_error{ "File "s + filename.string() +
                                  " does not point to a regular file"s };
}

const fs::path kDataFolder{ std::string{ DATA_FOLDER } };

}    // namespace

const fs::path UniqueWordsCounter::Utils::TextFiles::kEmpty{ kDataFolder / "empty.txt" };
const fs::path UniqueWordsCounter::Utils::TextFiles::kSample{ kDataFolder /
                                                              "sample.txt" };

const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticShortWords10MB{
    kDataFolder / "syntheticShortWords10MB.txt"
};
const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticLongWords10MB{
    kDataFolder / "syntheticLongWords10MB.txt"
};

const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticShortWords100MB{
    kDataFolder / "syntheticShortWords100MB.txt"
};
const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticLongWords100MB{
    kDataFolder / "syntheticLongWords100MB.txt"
};

const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticShortWords1000MB{
    kDataFolder / "syntheticShortWords1000MB.txt"
};
const fs::path UniqueWordsCounter::Utils::TextFiles::kSyntheticLongWords1000MB{
    kDataFolder / "syntheticLongWords1000MB.txt"
};

const fs::path UniqueWordsCounter::Utils::TextFiles::kEnglishWords{ kDataFolder /
                                                                    "englishWords.txt" };

const std::initializer_list<fs::path> UniqueWordsCounter::Utils::TextFiles::kAllFiles{
    kEmpty,
    kSample,
    kEnglishWords,
    kSyntheticShortWords10MB,
    kSyntheticLongWords10MB,
    kSyntheticShortWords100MB,
    kSyntheticLongWords100MB,
    kSyntheticShortWords1000MB,
    kSyntheticLongWords1000MB
};

auto UniqueWordsCounter::Utils::TextFiles::getFile(const fs::path &filename)
    -> std::ifstream
{
    using namespace std::string_literals;

    checkFilenameValid(filename);

    auto file = std::ifstream{ filename };

    if (!file.is_open())
        throw std::runtime_error{ "Cannot open file: "s + filename.string() };

    return file;
}

UniqueWordsCounter::Utils::TextFiles::WordsGenerator::WordsGenerator(
    std::initializer_list<fs::path> files)
    : _files{ files.begin(), files.end() }, _currentFileName{ _files.begin() }
{
    for (const auto &filename : _files)
        checkFilenameValid(filename);

    if (!_files.empty())
        _currentFile = getFile(*_currentFileName);

    advance();
}

void UniqueWordsCounter::Utils::TextFiles::WordsGenerator::advance()
{
    if (_currentFileName == _files.end())
        return _word.clear();

    if (_currentFile >> _word)
    {
        ++_wordCounter;
    }
    else
    {
        ++_currentFileName;
        if (_currentFileName != _files.end())
            _currentFile = getFile(_currentFileName->c_str());
        else
            _currentFile.close();
        advance();
    }
}

UniqueWordsCounter::Utils::TextFiles::WordsGenerator::iterator::iterator(
    WordsGenerator &instance)
    : _instance{ instance },
      _word{ std::move(_instance._word) },
      _wordCounter{ _instance._wordCounter }
{
}

UniqueWordsCounter::Utils::TextFiles::WordsGenerator::iterator::iterator(
    WordsGenerator &instance,
    std::nullptr_t)
    : _instance{ instance }
{
}

bool UniqueWordsCounter::Utils::TextFiles::WordsGenerator::iterator::operator==(
    const iterator &rhs) const
{
    return _wordCounter == rhs._wordCounter && ((&_instance) == (&rhs._instance));
}

auto UniqueWordsCounter::Utils::TextFiles::WordsGenerator::iterator::operator++()
    -> iterator &
{
    _instance.advance();

    _word        = std::move(_instance._word);
    _wordCounter = _word.empty() ? 0ULL : _instance._wordCounter;

    return *this;
}

auto UniqueWordsCounter::Utils::TextFiles::WordsGenerator::iterator::operator++(int)
    -> iterator
{
    auto self = (*this);
    ++(*this);
    return self;
}
