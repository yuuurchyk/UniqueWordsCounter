cmake_minimum_required(VERSION 3.20.1 FATAL_ERROR)

project(uniqueWordsCounter LANGUAGES CXX)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)

include(tbb)
include(argparse)

add_library(
    lib_${PROJECT_NAME}
    STATIC
    include/UniqueWordsCounter/methods.h
    include/UniqueWordsCounter/utils/wordsSet.h
    include/UniqueWordsCounter/utils/wordsSetImpl.h
    include/UniqueWordsCounter/utils/wordBucket.h
    include/UniqueWordsCounter/utils/hash.h
    include/UniqueWordsCounter/utils/textFiles.h
    include/UniqueWordsCounter/utils/scanningImpl.h
    include/UniqueWordsCounter/utils/itemManager.h
    include/UniqueWordsCounter/utils/itemManagerImpl.h
    src/methods.cpp
    src/method/baseline.cpp
    src/method/bufferScanning.cpp
    src/method/optimizedBaseline.cpp
    src/method/producerConsumer.cpp
    src/method/optimizedProducerConsumer.cpp
    src/method/concurrentSetProducerConsumer.cpp
    src/utils/hash/murmur64.cpp
    src/utils/hash/polynomial.cpp
    src/utils/textFiles.cpp
)
target_include_directories(lib_${PROJECT_NAME} PUBLIC include PRIVATE src)
target_link_libraries(lib_${PROJECT_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT} TBB::tbb)
target_compile_definitions(lib_${PROJECT_NAME} PRIVATE DATA_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_executable(${PROJECT_NAME} examples/uniqueWordsCounter.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE lib_${PROJECT_NAME} argparse::argparse)

add_executable(generateFile tools/generateFile.cpp)
target_link_libraries(generateFile PRIVATE argparse::argparse)

set(
    Targets
    lib_${PROJECT_NAME}
    ${PROJECT_NAME}
    generateFile
)

option(WithBenchmarks "Whether to build benchmarks" ON)
if(WithBenchmarks)
    include(benchmark)

    set(BenchmarkTargets "")

    # methods benchmarks
    foreach(words short long)
        set(executableName benchmark_${words}Words)

        add_executable(
            ${executableName}
            benchmarks/method.h
            benchmarks/${words}Words.cpp
        )

        list(APPEND BenchmarkTargets ${executableName})
    endforeach()

    foreach(utilityBenchmarkName wordsSet hash)
        set(executableName benchmark_${utilityBenchmarkName})

        add_executable(
            ${executableName}
            benchmarks/${utilityBenchmarkName}.cpp
        )

        list(APPEND BenchmarkTargets ${executableName})
    endforeach()

    foreach(target ${BenchmarkTargets})
        target_link_libraries(
            ${target}
            PRIVATE
            lib_${PROJECT_NAME}
            benchmark::benchmark
            benchmark::benchmark_main
        )
        target_include_directories(
            ${target}
            PRIVATE
            benchmarks
        )
    endforeach()

    list(APPEND Targets ${BenchmarkTargets})
endif()

option(WithTests "Example of programs related to data structures and algorithms used in the final solution" ON)
if(WithTests)
    include(googleTest)

    foreach(testName hash unorderedSetBuckets wordsSet methodsCorrectness)
        set(targetName test_${testName})

        add_executable(
            ${targetName}
            tests/${testName}.cpp
        )

        target_link_libraries(
            ${targetName}
            PRIVATE
            lib_${PROJECT_NAME}
            gtest_main
        )

        list(APPEND Targets ${targetName})
    endforeach()
endif()

set_target_properties(
    ${Targets}
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    foreach(target ${Targets})
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    foreach(target ${Targets})
        target_compile_options(${target} PRIVATE /W4)
    endforeach()
endif()
