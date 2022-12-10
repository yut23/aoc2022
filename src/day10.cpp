/******************************************************************************
 * File:        day10.cpp
 *
 * Author:      yut23
 * Created:     2022-12-10
 *****************************************************************************/

#include "lib.h"
#include <cassert>  // for assert
#include <cstdlib>  // for abs
#include <iostream> // for cout
#include <string>   // for string, getline
#include <vector>   // for vector

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string command;
    std::vector<std::string> image{};
    int X = 1;
    int next_X = X;
    int arg;
    int wait = 0;
    int sum = 0;
    for (int cycle = 1; cycle <= 240; ++cycle, --wait) {
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
        int beam_pos = (cycle - 1) % 40;
        if (beam_pos == 0) {
            image.emplace_back(40, ' ');
        }
        if (std::abs(X - beam_pos) <= 1) {
            image.back()[beam_pos] = '#';
        }
    }
    std::cout << sum << std::endl;
    for (const auto &line : image) {
        std::cout << line << std::endl;
    }
    return 0;
}
