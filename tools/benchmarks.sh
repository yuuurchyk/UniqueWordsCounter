#!/bin/sh

set -e

./benchmark_baseline_shortWords 2>&1 | tee --append ./out.txt
./benchmark_baseline_longWords 2>&1 | tee --append ./out.txt

./benchmark_customScanning_shortWords 2>&1 | tee --append ./out.txt
./benchmark_customScanning_longWords 2>&1 | tee --append ./out.txt

./benchmark_producerConsumer_shortWords 2>&1 | tee --append ./out.txt
./benchmark_producerConsumer_longWords 2>&1 | tee --append ./out.txt
