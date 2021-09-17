#include "UniqueWordsCounter/methods.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

auto UniqueWordsCounter::Method::Sequential::optimizedBaseline(
    const std::string &filename) -> size_t
{
    auto file = Utils::TextFiles::getFile(filename);

    auto uniqueWords = Utils::WordsSet{};
    auto longWords   = std::unordered_set<std::string>{};
    auto lastWord    = std::string{};

    auto buffer = Utils::Scanning::Buffer{ 1ULL << 20 };

    auto wordCallback = [&uniqueWords, &longWords](const char *text, size_t len)
    {
        if (uniqueWords.canEmplace(text, len)) [[likely]]
            uniqueWords.emplace(text, len);
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

    return uniqueWords.size();
}
