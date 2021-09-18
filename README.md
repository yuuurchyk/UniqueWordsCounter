# Unique Words Counter

This repository contains source code used in *Justifiably Complex Solution to a Dead Simple Concurrency Task* medium article.

TODO: add link to article.

## What Is It About?

This repository contains the collection of methods I experimented with when solving the problem of counting the number of unique words in a given file.

**Note:** many methods are not optimal in terms of either CPU utilization or performance. They are present for benchmarking purposes only. See the benchmark results below to identify the good ones.

TODO: add benchmark results

## Building

### Compiler

The project uses **C++20**, so the latest compiler is required. The following toolchains were tested:

| Compiler | OS | Bitness |
| - | - | - |
| **GCC** >= 10.3.0 | *Ubuntu 20.04* | x64 |
| **Clang** >= 12.0.0 | *Ubuntu 20.04* | x64 |
| **MSVC** v142 - VS 2019 | *Windows10* | x64 |

### Additional Libraries

The following libraries are used in the project:

* [*argparse*](https://github.com/p-ranav/argparse)
* [*TBB*](https://github.com/oneapi-src/oneTBB)
* [*google benchmark*](https://github.com/google/benchmark)
* [*google test*](https://github.com/google/googletest)

Assembling all of them does not require lots of cmake configure/build time, so they are fetched and built from source automatically.

### Cmake Options

The following options are available:

| Option Name | Default Value | Description |
|-----|--------|------|
| **WithBenchmarks** | *ON* | include *google benchmark* library and benchmark targets |
| **WithTests** | *ON* | include *google test* library and test targets |

### Known Issues

At the moment of writing, it is not possible to build *TBB* if you have spaces in the project path. I have already submitted an issue on [github](https://github.com/oneapi-src/oneTBB/issues/531). Hope it would be overcome soon.

## Before You Start

There is a utility target for generating artificial text files used in tests and benchmarks. You should have the files available on your machine for the benchmarks/tests to run fine:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target generateFile

./generateFile --path ./data/syntheticShortWords10MB.txt --bytes 10485760 --words-size-mean 5 --words-size-stddev 2 --seed 10
./generateFile --path ./data/syntheticLongWords10MB.txt --bytes 10485760 --words-size-mean 15 --words-size-stddev 5 --seed 10

./generateFile --path ./data/syntheticShortWords100MB.txt --bytes 104857600 --words-size-mean 5 --words-size-stddev 2 --seed 100
./generateFile --path ./data/syntheticLongWords100MB.txt --bytes 104857600 --words-size-mean 15 --words-size-stddev 5 --seed 100

./generateFile --path ./data/syntheticShortWords1000MB.txt --bytes 1048576000 --words-size-mean 5 --words-size-stddev 2 --seed 1000
./generateFile --path ./data/syntheticLongWords1000MB.txt --bytes 1048576000 --words-size-mean 15 --words-size-stddev 5 --seed 1000
```
