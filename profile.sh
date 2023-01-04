#!/bin/bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./lib.sh

day=$1
num=${2:-1}
log_file="profiling/logs/day${day}_${num}.folded"
svg_file="profiling/day${day}_${num}.svg"

mkdir -p profiling/logs
make_quiet "build/profile/day$day"
perf record -g --call-graph dwarf -F 99 hyperfine --shell=none "build/profile/day$day input/day$day/input.txt"
perf script --header -F comm,pid,tid,time,event,ip,sym,dso | inferno-collapse-perf --skip-after main > "$log_file"
inferno-flamegraph --inverted --deterministic "$log_file" > "$svg_file"
