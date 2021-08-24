# Unique Words Counter

This repository contains source code used in *Justifiably Complex Solution to a Dead Simple Concurrency Task* medium article. You can read it here.

TODO: add link to article.

## What Is It About?

This repository contains the collection of methods I experimented with when solving the problem of counting the number of unique words in a given file.

**Note:** many methods are not optimal in terms of either CPU utilization or performance. They are present for benchmarking purposes only. See the benchmark results below to identify the good ones.

**Note:** because of the task artificiality, the input files should be constrained to only lowercase english letters and space characters. However, it is not hard to extend the methods to handle arbitrary characters.

## Building

The following libraries are required to build the examples:

1. *Boost*
2. *TBB*
3. *google benchmark*
4. *google test*

The following options in cmake control whether to build each library together with the examples or link to a system-installed one:

| Option Name | Default Value | Description |
|-----|--------|------|
| *UseSystemBoost* | **ON** | Look for globally installed Boost (ON) or build in-place (OFF) |
| *WithBenchmarks* | **ON** | Whether to build benchmark targets |
| *WithTests* | **ON** | Example of programs related to data structures and algorithms used in the final solution |

Assembling *TBB*, *benchmark* and *google test* does not require lots of cmake configure/build time, so they are built from source together with the project.

As for *boost*, it is preferred to link against an installed libraries/headers if you have them.

Here is how to configure the project to download all the dependencies for you:

```bash
>>> mkdir build
>>> cd build
>>> cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Debug -DUseSystemBoost=OFF ..
```

### Issues

At the moment of writing, it is not possible to build *TBB* if you have spaces in the project path. I have already submitted an issue on github. Hope it would be overcome soon.

## Before You Start

There is a utility target for generating artificial text files for you. To replicate the benchmarks, you should have the files to run them on:

```bash
>>> ./build/generateFile --path ./data/syntheticShortWords10MB.txt --bytes 10485760 --words-size-mean 5 --words-size-stddev 2 --seed 10
>>> ./build/generateFile --path ./data/syntheticLongWords10MB.txt --bytes 10485760 --words-size-mean 15 --words-size-stddev 5 --seed 10

>>> ./build/generateFile --path ./data/syntheticShortWords100MB.txt --bytes 104857600 --words-size-mean 5 --words-size-stddev 2 --seed 100
>>> ./build/generateFile --path ./data/syntheticLongWords100MB.txt --bytes 104857600 --words-size-mean 15 --words-size-stddev 5 --seed 100

>>> ./build/generateFile --path ./data/syntheticShortWords1000MB.txt --bytes 1048576000 --words-size-mean 5 --words-size-stddev 2 --seed 1000
>>> ./build/generateFile --path ./data/syntheticLongWords1000MB.txt --bytes 1048576000 --words-size-mean 15 --words-size-stddev 5 --seed 1000
```

TODO: add benchmark results
