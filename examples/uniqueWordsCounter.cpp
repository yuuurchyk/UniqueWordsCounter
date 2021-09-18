#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include <argparse/argparse.hpp>

#include "UniqueWordsCounter/methods.h"

using namespace std::string_literals;

namespace
{
auto join(const std::initializer_list<std::string> &args,
          const std::string &                       separator = ", "s) -> std::string
{
    return std::accumulate(
        args.begin(),
        args.end(),
        ""s,
        [&separator](std::string target, const std::string &source) -> std::string
        { return target.empty() ? source : std::move(target) + separator + source; });
};

auto getSequentialMethod(const std::string &method)
    -> std::function<size_t(const std::string &)>
{
    using namespace UniqueWordsCounter::Method;
    using namespace UniqueWordsCounter::Method::Sequential;

    const auto kExecutionMap =
        std::unordered_map<std::string, std::function<size_t(const std::string &)>>{
            { kBaseline, baseline },
            { kBufferScanning, bufferScanning },
            { kOptimizedBaseline, optimizedBaseline }
        };

    return kExecutionMap.at(method);
}

auto getParallelMethod(const std::string &method)
    -> std::function<size_t(const std::string &, size_t)>
{
    using namespace UniqueWordsCounter::Method;
    using namespace UniqueWordsCounter::Method::Parallel;

    const auto kExecutionMap =
        std::unordered_map<std::string,
                           std::function<size_t(const std::string &, size_t)>>{
            { kProducerConsumer, producerConsumer },
            { kConcurrentSetProducerConsumer, concurrentSetProducerConsumer },
            { kOptimizedProducerConsumer, optimizedProducerConsumer }
        };

    return kExecutionMap.at(method);
}

auto determineNumberOfParallelJobs(int jobs) -> size_t
{
    const auto hardwareConcurrency = []() -> size_t
    {
        auto result = static_cast<size_t>(std::thread::hardware_concurrency());
        if (result == 0)
        {
            std::cerr
                << "Cannot determine hardware concurrent level of target device, using 1 thread."
                << std::endl;
            return 1;
        }
        else
        {
            return result;
        }
    }();

    if (jobs == -1)
        return hardwareConcurrency;
    if (jobs <= 0)
        throw std::runtime_error{ "Wrong number of jobs: "s + std::to_string(jobs) };

    if (jobs > hardwareConcurrency)
        std::cerr << "Number of jobs " << jobs
                  << " is greater than hardware concurrency level: "
                  << hardwareConcurrency << std::endl;

    return hardwareConcurrency;
}

auto getMethodImplementation(const std::string &method,
                             const std::string &filepath,
                             int                jobs) -> std::function<size_t()>
{
    using namespace UniqueWordsCounter::Method;

    if (std::find(kSequentialMethods.begin(), kSequentialMethods.end(), method) !=
        kSequentialMethods.end())
        return [filepath, method]() { return getSequentialMethod(method)(filepath); };
    if (std::find(kParallelMethods.begin(), kParallelMethods.end(), method) !=
        kParallelMethods.end())
        return [filepath, method, jobs]() {
            return getParallelMethod(method)(filepath,
                                             determineNumberOfParallelJobs(jobs));
        };
    throw std::runtime_error{ "Invalid method: "s + method };
}

auto getCurrentTime() -> std::chrono::high_resolution_clock::time_point
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto result = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return result;
}

}    // namespace

auto main(int argc, char **argv) -> int
{
    using namespace UniqueWordsCounter::Method;

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
        .help("Method to use."s + " Sequential methods: "s + join(kSequentialMethods) +
              ". Parallel methods: "s + join(kParallelMethods) + "."s);
    program.add_argument("-j", "--jobs")
        .scan<'i', int>()
        .default_value(-1)
        .help(
            "Number of parallel jobs (used in parallel algorithms, -1 is equivalent to using all available threads on the system)");

    program.parse_args(argc, argv);

    const auto &filepath = program.get("--filepath");
    const auto &method   = program.get("--method");
    const auto &jobs     = program.get<int>("--jobs");

    const auto methodImplementation = getMethodImplementation(method, filepath, jobs);

    const auto start  = getCurrentTime();
    const auto result = methodImplementation();
    const auto end    = getCurrentTime();

    std::cout << "Found " << result << " unique words\n";
    std::cout
        << "Elapsed "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " milliseconds" << std::endl;

    return EXIT_SUCCESS;
}
