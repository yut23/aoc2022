#!/bin/bash
set -euo pipefail
# run last command in a pipeline in the current shell environment, rather than a subshell
shopt -s lastpipe
if [[ $# -lt 2 ]]; then
  # year or day not specified on command line
  date +"%-d %Y" | read -r day year
fi
day=${1:-$day}
year=${2:-$year}
dest_dir="input/day$day"
mkdir -p "$dest_dir"
filename="$dest_dir/input.txt"
if ! [[ -e "$filename" ]]; then
  # download the input
  session=$(<.aoc_session)
  curl "https://adventofcode.com/$year/day/$day/input" --cookie "session=$session" -o "$filename"
else
  >&2 echo "Input for day $day already downloaded."
  exit 1
fi
