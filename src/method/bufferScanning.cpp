#include "UniqueWordsCounter/methods.h"

#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"

/**
 * @brief Sequential baseline with a custom scanning routine.
 *
 * Superstructure over UniqueWordsCounter::Method::Sequential::baseline.
 * Method still uses standard c++ containers to store the words,
 * but the scanning is done by manual inspection of big raw buffers
 * of characters.
 *
 * @param filepath - path to the file to count the number of unique words in
 * @return size_t - number of unique words in a file provided in \p filepath
 *
 * @see UniqueWordsCounter::Method::Sequential::baseline
 * @see UniqueWordsCounter::Utils::Buffer
 * @see UniqueWordsCounter::Utils::Scanning::bufferScanning
 */
auto UniqueWordsCounter::Method::Sequential::bufferScanning(
    const std::filesystem::path &filepath) -> size_t
{
    auto file = Utils::TextFiles::getFile(filepath);

    auto uniqueWords = std::unordered_set<std::string>{};
    auto lastWord    = std::string{};

    auto buffer = Utils::Scanning::Buffer{ 1ULL << 20 };

    auto wordCallback = [&uniqueWords](const char *text, size_t len)
    { uniqueWords.emplace(text, len); };

    auto lastWordCallback = [&lastWord](std::string &&lastBufferWord)
    { lastWord = std::move(lastBufferWord); };

    do
    {
        buffer.read(file);
        Utils::Scanning::bufferScanning(
            buffer, std::move(lastWord), wordCallback, lastWordCallback);
    } while (buffer.size() > 0);

    if (!lastWord.empty())
        uniqueWords.insert(std::move(lastWord));

    return uniqueWords.size();
}
