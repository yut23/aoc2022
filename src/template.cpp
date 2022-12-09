/******************************************************************************
 * File:        day{{DAY}}.cpp
 *
 * Author:      yut23
 * Created:     {{DATE}}
 *****************************************************************************/

#include "lib.h"
#include <iostream> // for cout
#include <string>   // for string, getline

namespace aoc::day{{DAY}} {

} // namespace aoc::day{{DAY}}

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        using namespace aoc::day{{DAY}};
        // do stuff
    }
    return 0;
}
