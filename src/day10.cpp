/******************************************************************************
 * File:        day10.cpp
 *
 * Author:      yut23
 * Created:     2022-12-10
 *****************************************************************************/

#include "lib.h"
#include <cassert>  // for assert
#include <iostream> // for cout
#include <string>   // for string, getline

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string command;
    int X = 1;
    int next_X = X;
    int arg;
    int wait = 0;
    int sum = 0;
    for (int cycle = 1; cycle <= 220; ++cycle, --wait) {
        if (wait == 0) {
            X = next_X;
            if (!(infile >> command)) {
                break;
            }
            if (command == "noop") {
                next_X = X;
                wait = 1;
            } else if (command == "addx") {
                infile >> arg;
                next_X = X + arg;
                wait = 2;
            } else {
                assert(false);
            }
        }
        if ((cycle - 20) % 40 == 0) {
            sum += cycle * X;
            if constexpr (aoc::DEBUG) {
                std::cerr << "Cycle " << cycle << ": X = " << X
                          << ", signal strength = " << cycle * X << "; sum now "
                          << sum << std::endl;
            }
        }
    }
    std::cout << sum << std::endl;
    return 0;
}
