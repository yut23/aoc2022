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

template <class T> void move(Stacks<T> &stacks, size_t src, size_t dst) {
  stacks[dst].push_back(stacks[src].back());
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

  // make a copy for part 2
  auto stacks_2{stacks};

  // handle instructions
  int count, src, dst;
  std::string _;
  while (infile >> _ >> count >> _ >> src >> _ >> dst) {
    // 1-based to 0-based
    --src;
    --dst;
    // part 1
    for (int i = 0; i < count; ++i) {
      move(stacks, src, dst);
    }
    // part 2
    {
      auto &source = stacks_2[src];
      auto &dest = stacks_2[dst];
      auto first = source.end();
      for (int i = 0; i < count; ++i, --first)
        ;
      dest.splice(dest.cend(), source, first, source.end());
    }
  }
  for (const auto &s : stacks) {
    std::cout << s.back();
  }
  std::cout << std::endl;
  for (const auto &s : stacks_2) {
    std::cout << s.back();
  }
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
