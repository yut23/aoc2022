/******************************************************************************
 * File:        day{{DAY}}.cpp
 *
 * Author:      yut23
 * Created:     {{DATE}}
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <string>

int main(int argc, char **argv) {
    auto infile = parse_args(argc, argv);

    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        // do stuff
    }
    return EXIT_SUCCESS;
}
