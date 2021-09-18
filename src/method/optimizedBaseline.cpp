#include "UniqueWordsCounter/methods.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

/**
 * @brief Sequential baseline with an optimized scanning routine and custom words
 * container.
 *
 * Superstructure over UniqueWordsCounter::Method::Sequential::bufferScanning.
 * In addition to custom scanning routine, the method also uses a custom version
 * of a string set, which is more optimal for everyday english words in terms of
 * cache locality and number of memory allocations when compared to a standard
 * c++ containers.
 *
 * @param filepath - path to the file to count the number of unique words in
 * @return size_t - number of unique words in a file provided in \p filepath
 *
 * @see UniqueWordsCounter::Method::Sequential::bufferScanning
 * @see UniqueWordsCounter::Utils::WordsSet
 */
auto UniqueWordsCounter::Method::Sequential::optimizedBaseline(
    const std::filesystem::path &filepath) -> size_t
{
    auto file = Utils::TextFiles::getFile(filepath);

    auto uniqueWords = Utils::WordsSet{};
    auto longWords   = std::unordered_set<std::string>{};
    auto lastWord    = std::string{};

    auto buffer = Utils::Scanning::Buffer{ 1ULL << 20 };

    auto wordCallback = [&uniqueWords, &longWords](const char *text, size_t len)
    {
        if (uniqueWords.canEmplace(text, len)) [[likely]]
            uniqueWords.emplace(
                text, static_cast<decltype(uniqueWords)::element_type::size_type>(len));
        else
            longWords.emplace(text, len);
    };

    auto lastWordCallback = [&lastWord](std::string &&lastBufferWord)
    { lastWord = std::move(lastBufferWord); };

    do
    {
        buffer.read(file);
        Utils::Scanning::bufferScanning(
            buffer, std::move(lastWord), wordCallback, lastWordCallback);
    } while (buffer.size() > 0);

    if (!lastWord.empty())
        wordCallback(lastWord.data(), lastWord.size());

    return uniqueWords.size() + longWords.size();
}
