#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>

#include <argparse/argparse.hpp>

using namespace std::string_literals;

namespace
{
auto generateFile(const std::filesystem::path &path,
                  size_t                       charactersNum,
                  double                       wordsSizemean,
                  double                       wordsSizeStddev,
                  size_t                       seed) -> void
{
    if (std::filesystem::exists(path))
        throw std::runtime_error{ "Path "s + path.string() + " already exists"s };

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
        throw std::runtime_error{ "Cannot open file: "s + path.string() };

    file << text;
}

}    // namespace

auto main(int argc, char **argv) -> int
{
    auto program = argparse::ArgumentParser{ "generateFile" };
    program.add_description(
        "Generates a file with random characters.\n"
        "You can either provide a filename to generate a file inside data folder or a full path");

    program.add_argument("-p", "--path").required().help("Path to the resulting file");
    program.add_argument("-b", "--bytes")
        .scan<'u', size_t>()
        .required()
        .help("Size of the resulting file, in bytes");
    program.add_argument("--words-size-mean")
        .scan<'g', double>()
        .required()
        .help("Size of the resulting file, in bytes");
    program.add_argument("--words-size-stddev")
        .scan<'g', double>()
        .required()
        .help("Standard deviation of word size in the resulting text");
    program.add_argument("-s", "--seed")
        .scan<'u', size_t>()
        .default_value(size_t{})
        .help("Seed to use for random letters generation. If 0, no seed is used");

    program.parse_args(argc, argv);

    const auto &wordsSizeMean   = program.get<double>("--words-size-mean");
    const auto &wordsSizeStddev = program.get<double>("--words-size-stddev");

    if (wordsSizeMean <= 0)
        throw std::runtime_error{ "Word size mean "s + std::to_string(wordsSizeMean) +
                                  " should be positive"s };
    if (wordsSizeStddev <= 0)
        throw std::runtime_error{ "Word size stddev "s + std::to_string(wordsSizeStddev) +
                                  " should be positive"s };
    if (wordsSizeMean - 2 * wordsSizeStddev < 0)
        throw std::runtime_error{ "Word size: mean - 2 * stddev should be positive" };

    generateFile(program.get("--path"),
                 program.get<size_t>("--bytes"),
                 program.get<double>("--words-size-mean"),
                 program.get<double>("--words-size-stddev"),
                 program.get<size_t>("--seed"));

    return EXIT_SUCCESS;
}
