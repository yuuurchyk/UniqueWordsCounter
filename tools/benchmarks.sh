#!/bin/sh

set -e

./benchmark_baseline_shortWords | tee --append ./out.txt
./benchmark_baseline_longWords | tee --append ./out.txt

./benchmark_customScanning_shortWords | tee --append ./out.txt
./benchmark_customScanning_longWords | tee --append ./out.txt

./benchmark_producerConsumer_shortWords | tee --append ./out.txt
./benchmark_producerConsumer_longWords | tee --append ./out.txt
