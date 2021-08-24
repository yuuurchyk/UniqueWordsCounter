#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>

namespace
{
namespace fs = std::filesystem;

auto generateFile(const fs::path &path,
                  size_t          charactersNum,
                  double          wordsSizemean,
                  double          wordsSizeStddev,
                  size_t          seed) -> void
{
    if (fs::exists(path))
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Path " << path << " already exists";
        throw std::runtime_error{ errorMessage.str() };
    }

    auto text = std::string{};
    text.reserve(charactersNum);

    constexpr const char *kLetters{ "abcdefghijklmnopqrstuvwxyz" };
    constexpr const char *kSpaces{ " \n\t" };

    const auto kLettersNum = std::strlen(kLetters);
    const auto kSpacesNum  = std::strlen(kSpaces);

    auto rd  = std::random_device{};
    auto gen = std::mt19937{};
    if (seed == 0)
        gen.seed(rd());
    else
        gen.seed(seed);

    std::normal_distribution<> wordLengthDistribution{ wordsSizemean, wordsSizeStddev };

    std::uniform_int_distribution<size_t> lettersDistribution{ 0, kLettersNum - 1 };
    std::uniform_int_distribution<size_t> spacesDistribution{ 0, kSpacesNum - 1 };

    while (text.size() < charactersNum)
    {
        auto wordSize = static_cast<long long>(std::round(wordLengthDistribution(gen)));
        wordSize      = std::max(wordSize, 1LL);

        while (wordSize--)
            text.push_back(kLetters[lettersDistribution(gen)]);
        text.push_back(kSpaces[spacesDistribution(gen)]);
    }

    while (text.size() > charactersNum)
        text.pop_back();

    auto file = std::ofstream{ path };
    if (!file.is_open())
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Cannot open file: " << path;
        throw std::runtime_error{ errorMessage.str() };
    }

    file << text;
}

}    // namespace

auto main(int argc, char **argv) -> int
{
    namespace po = boost::program_options;

    auto description = po::options_description{
        "Generates a file with random characters.\n"
        "You can either provide a filename to generate a file inside data folder or a full path"
    };

    // clang-format off
    description.add_options()
        (
            "help,h",
            "Shows help message"
        )
        (
            "path,p",
            po::value<std::string>()->required(),
            "Path to the resulting file"
        )
        (
            "bytes,b",
            po::value<size_t>()->required(),
            "Size of the resulting file, in bytes"
        )
        (
            "words-size-mean",
            po::value<double>()->required(),
            "Mean size of words in the resulting text"
        )
        (
            "words-size-stddev",
            po::value<double>()->required(),
            "Standard deviation of word size in the resulting text"
        )
        (
            "seed,s",
            po::value<size_t>()->default_value(0),
            "Seed to use for random letters generation. If 0, no seed is used"
        );
    // clang-format on

    auto variables = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, description), variables);

    if (variables.count("help"))
    {
        std::cout << description;
        return EXIT_SUCCESS;
    }

    po::notify(variables);

    const auto path            = variables["path"].as<std::string>();
    const auto bytes           = variables["bytes"].as<size_t>();
    const auto seed            = variables["seed"].as<size_t>();
    const auto wordsSizeMean   = variables["words-size-mean"].as<double>();
    const auto wordsSizeStddev = variables["words-size-stddev"].as<double>();

    if (wordsSizeMean < 0)
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Word size mean should be positive, got " << wordsSizeMean;
        throw std::runtime_error{ errorMessage.str() };
    }
    if (wordsSizeStddev < 0)
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Word size stddev should be positive, got " << wordsSizeStddev;
        throw std::runtime_error{ errorMessage.str() };
    }
    if (wordsSizeMean - 2 * wordsSizeStddev < 0)
    {
        throw std::runtime_error{ "Word size: mean - 2 * stddev should be positive" };
    }

    generateFile(path, bytes, wordsSizeMean, wordsSizeStddev, seed);

    return EXIT_SUCCESS;
}
