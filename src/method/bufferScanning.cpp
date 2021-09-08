#include "UniqueWordsCounter/methods.h"

#include <cctype>
#include <cstring>
#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
template <typename T>
concept HashMap = requires(T t)
{
    std::is_default_constructible_v<T>;
    { t.emplace("a", size_t{ 1ULL }) };
    { t.insert(std::string{ "a" }) };

    // clang-format off
    { t.size() } -> std::same_as<size_t>;
    // clang-format on
};

template <HashMap T>
auto bufferScanningImpl(const std::string &filename) -> size_t
{
    using namespace UniqueWordsCounter::Utils;

    auto file = TextFiles::getFile(filename);

    auto uniqueWords = T{};
    auto lastWord    = std::string{};

    static constexpr auto kBufferSize = size_t{ 1ULL << 20 };
    auto                  buffer      = Scanning::Buffer{ kBufferSize };

    do
    {
        buffer.read(file);

        bufferScanning(
            buffer,
            std::move(lastWord),
            [&uniqueWords](const char *text, size_t size)
            { uniqueWords.emplace(text, size); },
            [&lastWord](std::string &&lastBufferWord)
            { lastWord = std::move(lastBufferWord); });
    } while (buffer.size() > 0);

    if (!lastWord.empty())
        uniqueWords.insert(std::move(lastWord));

    return uniqueWords.size();
}

}    // namespace

auto UniqueWordsCounter::Method::bufferScanning(const std::string &filename) -> size_t
{
    return bufferScanningImpl<std::unordered_set<std::string>>(filename);
}

auto UniqueWordsCounter::Method::optimizedBaseline(const std::string &filename) -> size_t
{
    return bufferScanningImpl<Utils::OpenAddressingSet>(filename);
}
