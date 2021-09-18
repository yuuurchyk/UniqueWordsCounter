#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>

#include "gtest/gtest.h"

#include "UniqueWordsCounter/methods.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace fs = std::filesystem;
using namespace UniqueWordsCounter::Method;
using namespace UniqueWordsCounter::Method::Sequential;
using namespace UniqueWordsCounter::Method::Parallel;

namespace
{
auto getBaselineResult(const fs::path &filepath) -> size_t
{
    static auto results = std::unordered_map<std::string, size_t>{};

    if (!results.contains(filepath.string()))
        results.insert({ filepath.string(), baseline(filepath) });

    return results.at(filepath.string());
}

}    // namespace

using Function_type     = std::function<size_t(const fs::path &)>;
using FunctionData_type = std::tuple<std::string, Function_type>;
using TestParam_type    = std::tuple<fs::path, FunctionData_type>;

class MethodsCorrectnessFixture : public ::testing::TestWithParam<TestParam_type>
{
};

TEST_P(MethodsCorrectnessFixture, ResultsComparison)
{
    const auto &[filepath, functionData] = GetParam();
    const auto &[functionName, function] = functionData;

    const auto &baselineResult = getBaselineResult(filepath);
    const auto &functionResult = function(filepath);

    ASSERT_EQ(baselineResult, functionResult)
        << "Method " << functionName
        << " has produced incorrect number of words: " << functionResult
        << ". Baseline result: " << baselineResult;
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    ,
    MethodsCorrectnessFixture,
    ::testing::Combine(
        ::testing::ValuesIn(UniqueWordsCounter::Utils::TextFiles::kAllFiles),
        ::testing::ValuesIn({
            FunctionData_type{ kBufferScanning, bufferScanning },
            FunctionData_type{ kOptimizedBaseline, optimizedBaseline },
            FunctionData_type{ kProducerConsumer + "3jobs", [](const fs::path &path){ return producerConsumer(path, 3); } },
            FunctionData_type{ kProducerConsumer + "4jobs", [](const fs::path &path){ return producerConsumer(path, 4); } },
            FunctionData_type{ kProducerConsumer + "6jobs", [](const fs::path &path){ return producerConsumer(path, 6); } },
            FunctionData_type{ kProducerConsumer + "10jobs", [](const fs::path &path){ return producerConsumer(path, 10); } },
            FunctionData_type{ kConcurrentSetProducerConsumer + "3jobs", [](const fs::path &path){ return concurrentSetProducerConsumer(path, 3); } },
            FunctionData_type{ kConcurrentSetProducerConsumer + "4jobs", [](const fs::path &path){ return concurrentSetProducerConsumer(path, 4); } },
            FunctionData_type{ kConcurrentSetProducerConsumer + "6jobs", [](const fs::path &path){ return concurrentSetProducerConsumer(path, 6); } },
            FunctionData_type{ kConcurrentSetProducerConsumer + "10jobs", [](const fs::path &path){ return concurrentSetProducerConsumer(path, 10); } },
            FunctionData_type{ kOptimizedProducerConsumer + "3jobs", [](const fs::path &path){ return optimizedProducerConsumer(path, 3); } }
        })
    ),
    [](const ::testing::TestParamInfo<MethodsCorrectnessFixture::ParamType> &info)
    {
        const auto &filePath     = fs::path{ std::get<0>(info.param) };
        const auto &functionName = std::get<0>(std::get<1>(info.param));

        const auto &filename = filePath.filename().replace_extension("").string();

        return functionName + "_" + filename;
    });
// clang-format on

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
