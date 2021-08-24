#include <sstream>
#include <stdexcept>

#include "UniqueWordsCounter/utils/getFile.h"

auto getFile(const char *filename) -> std::ifstream
{
    if (filename == nullptr)
        throw std::runtime_error{ "Filename cannot be nullptr" };

    auto file = std::ifstream{ filename };

    if (!file.is_open())
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Cannot open file: " << filename;
        throw std::runtime_error{ errorMessage.str() };
    }

    return file;
}
