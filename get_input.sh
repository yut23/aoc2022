#!/bin/bash
set -euo pipefail
curr_date=($(date +"%Y %-d"))
day=${1:-${curr_date[1]}}
year=${curr_date[0]}
session=$(<.aoc_session)
curl "https://adventofcode.com/2021/day/$day/input" --cookie "session=$session" -o "input/day$day.txt"
