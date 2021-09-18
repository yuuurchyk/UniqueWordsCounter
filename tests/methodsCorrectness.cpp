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
auto getBaselineResult(const std::filesystem::path &filepath) -> size_t
{
    static auto results = std::unordered_map<std::string, size_t>{};

    if (!results.contains(filepath))
        results.insert(
            { filepath, UniqueWordsCounter::Method::Sequential::baseline(filepath) });

    return results.at(filepath);
}

}    // namespace

using FunctionData_type =
    std::tuple<std::string, std::function<size_t(std::filesystem::path)>>;
using TestParam_type = std::tuple<std::filesystem::path, FunctionData_type>;

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

using namespace std::string_literals;

INSTANTIATE_TEST_SUITE_P(
    ,
    MethodsCorrectnessFixture,
    ::testing::Combine(
        ::testing::ValuesIn(UniqueWordsCounter::Utils::TextFiles::kAllFiles),
        ::testing::ValuesIn({
            std::make_tuple("bufferScanning"s,
                            std::function<size_t(std::filesystem::path)>{
                                UniqueWordsCounter::Method::Sequential::bufferScanning }),
            std::make_tuple(
                "optimizedBaseline"s,
                std::function<size_t(std::filesystem::path)>{
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
