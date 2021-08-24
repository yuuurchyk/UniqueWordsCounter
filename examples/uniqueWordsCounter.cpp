#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <boost/program_options.hpp>

#include "UniqueWordsCounter/methods.h"

auto main(int argc, char **argv) -> int
{
    namespace po = boost::program_options;

    // methods representation
    // sequential
    constexpr auto kBaseline       = "baseline";
    constexpr auto kCustomScanning = "customScanning";

    // parallel
    constexpr auto kProducerConsumer = "producerConsumer";

    auto description = po::options_description{
        "Calculates number of unique words in the provided file.\n"
        "Note that the file should contain only lowercase english letters and space characters.\n"
        "Some methods might not work correctly otherwise."
    };

    const auto methodDescription =
        [&kBaseline, &kCustomScanning, &kProducerConsumer]() -> std::string
    {
        auto description = std::stringstream{};

        description << "Method to use. "
                    << "Sequential methods: " << kBaseline << ", " << kCustomScanning
                    << ". Parallel methods: " << kProducerConsumer;

        return description.str();
    }();

    // clang-format off
    description.add_options()
        (
            "help,h",
            "Shows help message"
        )
        (
            "filepath,f",
            po::value<std::string>()->required(),
            "Path to the text file to read"
        )
        (
            "method,m",
            po::value<std::string>()->required(),
            methodDescription.c_str()
        );
    // clang-format on

    auto variables = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, description), variables);

    if (variables.count("help"))
    {
        std::cout << description << std::endl;
        return EXIT_SUCCESS;
    }

    po::notify(variables);

    const auto filepath = variables["filepath"].as<std::string>();
    const auto method   = variables["method"].as<std::string>();

    auto result = size_t{};

    if (method == kBaseline)
        result = UniqueWordsCounter::Sequential::baseline(filepath.c_str());
    else if (method == kCustomScanning)
        result = UniqueWordsCounter::Sequential::customScanning(filepath.c_str());
    else if (method == kProducerConsumer)
        result = UniqueWordsCounter::Parallel::producerConsumer(
            filepath.c_str(), std::max(0U, std::thread::hardware_concurrency()));
    else
        throw std::runtime_error{ "Invalid method: " + method };

    std::cout << "Found " << result << " unique words" << std::endl;

    return EXIT_SUCCESS;
}
