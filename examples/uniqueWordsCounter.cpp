// TODO: add logging of time measurements
// TODO: add parallelism level option

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include <argparse/argparse.hpp>

#include "UniqueWordsCounter/methods.h"

using namespace std::string_literals;

namespace
{
auto join(const std::initializer_list<std::string> &args) -> std::string
{
    static const auto separator = ", "s;

    auto result = std::stringstream{};

    for (auto it = args.begin(); it != args.end(); ++it)
    {
        result << *it;
        if (std::next(it) != args.end())
            result << separator;
    }

    return result.str();
};

}    // namespace

auto main(int argc, char **argv) -> int
{
    using namespace UniqueWordsCounter::Method;
    using namespace UniqueWordsCounter::Method::Sequential;
    using namespace UniqueWordsCounter::Method::Parallel;

    auto program = argparse::ArgumentParser{ "uniqueWordsCounter" };
    program.add_description("Calculates number of unique words in the provided file");

    program.add_argument("-f", "--filepath")
        .required()
        .action(
            [](const std::string &filepath)
            {
                if (!std::filesystem::is_regular_file(filepath))
                    throw std::runtime_error{ filepath +
                                              " does not point to a regular file"s };
                return filepath;
            })
        .help("Path to the text file to read");
    program.add_argument("-m", "--method")
        .required()
        .action(
            [](const std::string &method)
            {
                if (std::find(kAllMethods.begin(), kAllMethods.end(), method) ==
                    kAllMethods.end())
                    throw std::runtime_error{ "Invalid method: "s + method +
                                              ". Allowed values: "s + join(kAllMethods) };
                return method;
            })
        .help("Method to use. Allowed values: "s + join(kAllMethods));

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to parse command line arguments:\n";
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    const auto &filepath = program.get("--filepath");
    const auto &method   = program.get("--method");

    // TODO: refactor
    const auto kExecutionMap = []()
        -> std::unordered_map<std::string, std::function<size_t(const std::string &)>>
    {
        using namespace UniqueWordsCounter::Method;

        auto result =
            std::unordered_map<std::string, std::function<size_t(const std::string &)>>{};

        result[kBaseline]          = baseline;
        result[kBufferScanning]    = bufferScanning;
        result[kOptimizedBaseline] = optimizedBaseline;

        const auto threadsNum = std::thread::hardware_concurrency() == 0U ?
                                    1U :
                                    std::thread::hardware_concurrency();

        result[kProducerConsumer] = [&threadsNum](const std::string &filepath)
        { return producerConsumer(filepath, std::max(1U, threadsNum - 1U)); };
        result[kOptimizedProducerConsumer] = [&threadsNum](const std::string &filepath)
        { return optimizedProducerConsumer(filepath, std::max(1U, threadsNum - 1U)); };
        result[kConcurrentSetProducerConsumer] = [&threadsNum](
                                                     const std::string &filepath) {
            return concurrentSetProducerConsumer(filepath, std::max(1U, threadsNum - 1U));
        };

        return result;
    }();

    if (!kExecutionMap.contains(method))
        throw std::logic_error{ "Method "s + method + " not recognized."s };

    const auto result = kExecutionMap.at(method)(filepath);
    std::cout << "Found " << result << " unique words" << std::endl;

    return EXIT_SUCCESS;
}
