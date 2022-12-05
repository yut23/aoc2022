/******************************************************************************
 * File:        day5.cpp
 *
 * Author:      yut23
 * Created:     2022-12-05
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <list>
#include <string>
#include <vector>

template <class T> using Stacks = std::vector<std::list<T>>;

template <class T> void move(Stacks<T> &stacks, size_t src, size_t dest) {
  // 1-based to 0-based
  --src;
  --dest;
  stacks[dest].push_back(stacks[src].back());
  stacks[src].pop_back();
}

int main(int argc, char **argv) {
  auto infile = parse_args(argc, argv);

  // read and parse initial stacks
  Stacks<char> stacks{};
  std::string line;
  while (std::getline(infile, line)) {
    if (line.empty())
      break;
    if (stacks.empty()) {
      // line has length `3*num_stacks + (num_stacks - 1)`
      stacks.resize((line.length() + 1) / 4);
    }
    // insert new elements at the front (bottom) of the stack, since we're
    // parsing from the top down
    for (size_t i = 0; i < line.length(); i += 4) {
      if (line[i] == '[') {
        stacks[i / 4].push_front(line[i + 1]);
      }
    }
  }

  // handle instructions
  int count, src, dest;
  std::string _;
  while (infile >> _ >> count >> _ >> src >> _ >> dest) {
    for (int i = 0; i < count; ++i) {
      move(stacks, src, dest);
    }
  }
  for (const auto &s : stacks) {
    std::cout << s.back();
  }
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
