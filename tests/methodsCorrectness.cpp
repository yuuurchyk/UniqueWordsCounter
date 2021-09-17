#include "gtest/gtest.h"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>

#include "UniqueWordsCounter/methods.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
size_t getBaselineResult(const std::string &filename)
{
    static auto results = std::unordered_map<std::string, size_t>{};

    if (!results.contains(filename))
        results.insert(
            { filename, UniqueWordsCounter::Method::Sequential::baseline(filename) });

    return results.at(filename);
}

}    // namespace

using FunctionData_t = std::tuple<std::string, std::function<size_t(std::string)>>;
using TestParam_t    = std::tuple<std::string, FunctionData_t>;

class MethodsCorrectnessFixture : public ::testing::TestWithParam<TestParam_t>
{
};

TEST_P(MethodsCorrectnessFixture, ResultsComparison)
{
    const auto &[filename, functionData] = GetParam();
    const auto &[functionName, function] = functionData;

    const auto &baselineResult = getBaselineResult(filename);
    const auto &functionResult = function(filename);

    ASSERT_EQ(baselineResult, functionResult)
        << "Method " << functionName
        << " has produced incorrect number of words: " << functionResult
        << ". Baseline result: " << baselineResult;
}

using namespace std::string_literals;

INSTANTIATE_TEST_SUITE_P(
    ,
    MethodsCorrectnessFixture,
    ::testing::Combine(
        ::testing::ValuesIn(UniqueWordsCounter::Utils::TextFiles::kAllFiles),
        ::testing::ValuesIn({
            std::make_tuple("bufferScanning"s,
                            std::function<size_t(std::string)>{
                                UniqueWordsCounter::Method::Sequential::bufferScanning }),
            std::make_tuple(
                "optimizedBaseline"s,
                std::function<size_t(std::string)>{
                    UniqueWordsCounter::Method::Sequential::optimizedBaseline })
            // TODO: add other methods
        })),
    [](const ::testing::TestParamInfo<MethodsCorrectnessFixture::ParamType> &info)
    {
        const auto &filePath     = std::filesystem::path{ std::get<0>(info.param) };
        const auto &functionName = std::get<0>(std::get<1>(info.param));

        const auto &filename = filePath.filename().replace_extension("").string();

        return functionName + "_" + filename;
    });

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
