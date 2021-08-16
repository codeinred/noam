#!/bin/bash
commit="$(git rev-parse --short=12 HEAD)"
project_root="$(dirname $0)"
cd "$project_root"

bash build.sh "${@}"
build/bench \
    --benchmark_min_time=0.1 \
    --benchmark_repetitions=100 \
    --benchmark_enable_random_interleaving=true \
    --benchmark_display_aggregates_only=true \
    --benchmark_out_format=csv \
    --benchmark_out="bench/data/${commit}.csv"
