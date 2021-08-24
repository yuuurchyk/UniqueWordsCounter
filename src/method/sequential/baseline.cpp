#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/methods.h"
#include "UniqueWordsCounter/utils/getFile.h"

auto UniqueWordsCounter::Sequential::baseline(const char *filename) -> size_t
{
    auto file = getFile(filename);

    auto uniqueWords = std::unordered_set<std::string>{};

    for (auto word = std::string{}; file >> word;)
        uniqueWords.insert(std::move(word));

    return uniqueWords.size();
}
