#!/bin/bash
set -euo pipefail
# run last command in a pipeline in the current shell environment, rather than a subshell
shopt -s lastpipe
if [[ $# -lt 2 ]]; then
  # year or day not specified on command line
  date +"%-d %Y" | read -r day year
fi
day=${1:-$day}
# remove leading zero
day=${day#0}
# pad with zeros to two digits
day_padded=$(printf '%02d' "$day")
year=${2:-$year}
dest_dir="input/day$day_padded"
mkdir -p "$dest_dir"
filename="$dest_dir/input.txt"
if ! [[ -e "$filename" ]]; then
  # download the input
  session=$(<.aoc_session)
  url="https://adventofcode.com/$year/day/$day/input"
  exit_code=0
  curl --fail "$url" --cookie "session=$session" -o "$filename" || exit_code=$?
  if [[ $exit_code -ne 0 ]]; then
    >&2 echo "Failed to download $url"
    exit $exit_code
  fi
else
  >&2 echo "Input for day $day already downloaded."
  exit 1
fi
