/******************************************************************************
 * File:        day6.cpp
 *
 * Author:      yut23
 * Created:     2022-12-06
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <string>

int main(int argc, char **argv) {
  auto infile = parse_args(argc, argv);

  // read file line-by-line
  std::string line;
  infile >> line;
  int i;
  for (i = 3; i < line.length(); ++i) {
    // clang-format off
    if (line[i-3] != line[i-2] && line[i-3] != line[i-1] && line[i-3] != line[i] &&
        line[i-2] != line[i-1] && line[i-2] != line[i] &&
        line[i-1] != line[i])
      // clang-format on
      break;
  }
  std::cout << i + 1 << std::endl;
  return EXIT_SUCCESS;
}
