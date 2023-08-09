/******************************************************************************
 * File:        day05.cpp
 *
 * Author:      yut23
 * Created:     2022-12-05
 *****************************************************************************/

#include "lib.h"
#include <cstddef>  // for size_t
#include <iostream> // for cout
#include <list>     // for list
#include <string>   // for string, getline
#include <vector>   // for vector

namespace aoc::day5 {

template <class T>
using Stacks = std::vector<std::list<T>>;

template <class T>
void move(Stacks<T> &stacks, int src, int dst) {
    stacks[dst].push_back(stacks[src].back());
    stacks[src].pop_back();
}

} // namespace aoc::day5

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    // read and parse initial stacks
    aoc::day5::Stacks<char> stacks{};
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
        for (std::size_t i = 0; i < line.length(); i += 4) {
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
            aoc::day5::move(stacks, src, dst);
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
    return 0;
}
