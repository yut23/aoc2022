/******************************************************************************
 * File:        day3.cpp
 *
 * Author:      yut23
 * Created:     2022-12-03
 *****************************************************************************/

#include "lib.h"
#include <algorithm>  // for sort, set_union, set_intersection
#include <functional> // for less
#include <iostream>   // for cout, cerr
#include <iterator>   // for back_inserter
#include <string>     // for string, getline
#include <utility>    // for move
#include <vector>     // for vector

namespace aoc::day3 {

int calc_priority(char item) {
    if (item >= 'a' && item <= 'z') {
        return item - 'a' + 1;
    } else if (item >= 'A' && item <= 'Z') {
        return item - 'A' + 1 + 26;
    }
    return 0;
}

struct Rucksack {
    std::string first;
    std::string second;
    std::string combined{};

    Rucksack() : first{}, second{} {};

    explicit Rucksack(std::string &line) {
        int size = line.length() / 2;
        second = line.substr(size, size);
        first = line.substr(0, size);
        first.resize(size);
        std::ranges::sort(first);
        std::ranges::sort(second);
        std::ranges::set_union(first, second, std::back_inserter(combined));
    }
};

} // namespace aoc::day3

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string line;
    int total_1 = 0, total_2 = 0;
    int group_size = 0;
    std::string badge_options{};
    while (std::getline(infile, line)) {
        using namespace aoc::day3;
        Rucksack sack{line};
        std::vector<char> shared{};
        std::ranges::set_intersection(sack.first, sack.second,
                                      std::back_inserter(shared));
        total_1 += calc_priority(shared[0]);
        group_size += 1;
        if (group_size == 1) {
            badge_options = sack.combined;
        } else {
            std::string temp{};
            std::ranges::set_intersection(badge_options, sack.combined,
                                          std::back_inserter(temp));
            badge_options = std::move(temp);
        }
        if (group_size == 3) {
            if constexpr (aoc::DEBUG) {
                std::cerr << badge_options << std::endl;
            }
            total_2 += calc_priority(badge_options[0]);
            badge_options.clear();
            group_size = 0;
        }
    }
    std::cout << total_1 << std::endl;
    std::cout << total_2 << std::endl;
    return 0;
}
