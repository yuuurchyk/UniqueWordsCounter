#include "UniqueWordsCounter/methods.h"

#include <string>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/textFiles.h"

/**
 * @brief Sequential baseline
 *
 * Uses standard c++ file stream to read the words and
 * standard c++ containers to store them.
 *
 * @param filepath - path to the file to count the number of unique words in
 * @return size_t - number of unique words in a file provided in \p filepath
 */
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
