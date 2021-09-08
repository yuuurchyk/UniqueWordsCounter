cmake_minimum_required(VERSION 3.20.1 FATAL_ERROR)

project(uniqueWordsCounter LANGUAGES CXX)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)

option(AddGprofFlags "Whether to compile with -pg flag" OFF)
if(AddGprofFlags)
    message(STATUS "Building for profiling with gprof")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

option(UseSystemBoost "Look for globally installed Boost (ON) or build in-place (OFF)" ON)
if(UseSystemBoost)
    find_package(Boost REQUIRED COMPONENTS program_options iostreams)
else()
    include(boost)
endif()

include(tbb)

add_library(
    lib_${PROJECT_NAME}
    STATIC
    include/UniqueWordsCounter/methods.h
    include/UniqueWordsCounter/utils/openAddressingSet.h
    include/UniqueWordsCounter/utils/hash.h
    include/UniqueWordsCounter/utils/textFiles.h
    include/UniqueWordsCounter/utils/scanning.h
    src/method/sequential/baseline.cpp
    src/method/sequential/bufferScanning.cpp
    src/method/parallel/producerConsumer.cpp
    src/utils/openAddressingSet.cpp
    src/utils/hash/murmur64.cpp
    src/utils/hash/polynomial.cpp
    src/utils/textFiles.cpp
    src/utils/scanning.cpp
)
target_include_directories(lib_${PROJECT_NAME} PUBLIC include PRIVATE src)
target_link_libraries(lib_${PROJECT_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT} TBB::tbb)
target_compile_definitions(lib_${PROJECT_NAME} PRIVATE DATA_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_executable(${PROJECT_NAME} examples/uniqueWordsCounter.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE lib_${PROJECT_NAME} Boost::program_options)

add_executable(generateFile tools/generateFile.cpp)
target_link_libraries(generateFile PRIVATE Boost::program_options)

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

    foreach(utilityBenchmarkName openAddressingSet hash)
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

    foreach(testName hash unorderedSetBuckets openAddressingSet methodsCorrectness)
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
