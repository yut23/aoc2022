/******************************************************************************
 * File:        day1.cpp
 *
 * Author:      yut23
 * Created:     2022-12-01
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <string>
#include <vector>

int main(int argc, char **argv) {
  auto infile = parse_args(argc, argv);

  // read file line-by-line
  std::string line;
  std::vector<int> calories{0};
  while (std::getline(infile, line)) {
    if (line.empty()) {
      // new elf
      calories.push_back(0);
    } else {
      // parse calories for this item
      calories.back() += std::stoi(line);
    }
  }
  // sort in descending order
  std::ranges::sort(calories, std::greater<int>());
  // part 1
  std::cout << calories[0] << std::endl;
  // part 2
  std::cout << std::accumulate(calories.begin(), calories.begin() + 3, 0)
            << std::endl;
  return EXIT_SUCCESS;
}
