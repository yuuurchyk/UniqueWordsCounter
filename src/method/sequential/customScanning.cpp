#include "UniqueWordsCounter/methods.h"

#include <cctype>
#include <cstring>
#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
template <typename T>
concept HashMap = requires(T t)
{
    std::is_default_constructible_v<T>;
    { t.emplace("a", size_t{ 1 }) };
    { t.insert(std::string{ "a" }) };

    // clang-format off
    { t.size() } -> std::same_as<size_t>;
    // clang-format oon
};

template<HashMap T>
auto customScanningImpl(const std::string &filename) -> size_t
{
    auto file = getFile(filename);

    constexpr auto        kBufferSize = size_t{ 1'000'000 };
    constexpr const char *kSpaceCharacters{ " \n\t\r\f\v" };

    auto _arrOwner = std::unique_ptr<char[]>{ new char[kBufferSize + 5] };

    auto arr = _arrOwner.get();
    arr[0]   = 'a';
    arr[1]   = ' ';

    auto buffer = arr + 2;

    auto uniqueWords = T{};
    auto lastWord    = std::string{};

    do
    {
        file.read(buffer, kBufferSize);
        const auto charactersRead = file.gcount();

        const char *start{ buffer };
        const char *end{ start + charactersRead };

        buffer[charactersRead]     = ' ';
        buffer[charactersRead + 1] = 'a';
        buffer[charactersRead + 2] = '\0';

        // determine boundaries of the first and the last
        // words that may be shared among chunks
        auto firstWordStart = start;
        auto firstWordEnd   = std::strpbrk(firstWordStart, kSpaceCharacters);

        auto lastWordEnd   = end;
        auto lastWordStart = lastWordEnd - 1;
        while (!std::isspace(*lastWordStart))
            --lastWordStart;
        ++lastWordStart;

        // PART 1
        // handle first and last words separately
        lastWord.reserve(lastWord.size() + (firstWordEnd - firstWordStart));
        lastWord.append(firstWordStart, firstWordEnd - firstWordStart);

        // first word does not span across all the chunk
        if (firstWordEnd != end)
        {
            // add first word
            uniqueWords.insert(std::move(lastWord));

            // form last word
            // don't add it to unique words
            // since it may continue in the next chunk
            lastWord.clear();
            lastWord.reserve(lastWordEnd - lastWordStart);
            lastWord.append(lastWordStart, lastWordEnd - lastWordStart);
        }

        // PART 2
        // handle full words inside the buffer
        while (std::isspace(*start))
            ++start;

        // we start with letter at each iteration
        while (start < end)
        {
            auto nextSpace = std::strpbrk(start, kSpaceCharacters);

            uniqueWords.emplace(start,
                                static_cast<std::string::size_type>(nextSpace - start));

            start = nextSpace + 1;
            [[unlikely]] while (std::isspace(*start))++ start;
        }

    } while (file.gcount() > 0);

    uniqueWords.insert(std::move(lastWord));
    uniqueWords.insert(std::string{});

    return uniqueWords.size() - 1;
}

}    // namespace

auto UniqueWordsCounter::Sequential::customScanning(const std::string &filename) -> size_t
{
    return customScanningImpl<std::unordered_set<std::string>>(filename);
}

auto UniqueWordsCounter::Sequential::optimizedBaseline(const std::string &filename) -> size_t
{
    return customScanningImpl<OpenAddressingSet>(filename);
}
