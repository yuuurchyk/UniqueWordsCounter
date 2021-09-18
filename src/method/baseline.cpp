#include "UniqueWordsCounter/methods.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/textFiles.h"

auto UniqueWordsCounter::Method::Sequential::baseline(
    const std::filesystem::path &filepath) -> size_t
{
    auto file = Utils::TextFiles::getFile(filepath);

    auto uniqueWords = std::unordered_set<std::string>{};

    auto word = std::string{};
    while (file >> word)
        uniqueWords.insert(std::move(word));

    return uniqueWords.size();
}
