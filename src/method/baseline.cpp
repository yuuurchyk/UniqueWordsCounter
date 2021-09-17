#include "UniqueWordsCounter/methods.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/textFiles.h"

auto UniqueWordsCounter::Method::Sequential::baseline(const std::string &filename)
    -> size_t
{
    auto file = Utils::TextFiles::getFile(filename);

    auto uniqueWords = std::unordered_set<std::string>{};

    auto word = std::string{};
    while (file >> word)
        uniqueWords.insert(std::move(word));

    return uniqueWords.size();
}
