#include "UniqueWordsCounter/methods.h"

#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"

auto UniqueWordsCounter::Method::baseline(const std::string &filename) -> size_t
{
    auto file = Utils::TextFiles::getFile(filename);

    auto uniqueWords = std::unordered_set<std::string>{};

    Utils::Scanning::wordsScanning(file,
                                   [&uniqueWords](std::string &&word)
                                   { uniqueWords.insert(std::move(word)); });

    return uniqueWords.size();
}
