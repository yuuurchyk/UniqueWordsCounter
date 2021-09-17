#include "UniqueWordsCounter/methods.h"

#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"

auto UniqueWordsCounter::Method::Sequential::bufferScanning(const std::string &filename)
    -> size_t
{
    auto file = Utils::TextFiles::getFile(filename);

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
