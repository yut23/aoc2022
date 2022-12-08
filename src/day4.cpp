/******************************************************************************
 * File:        day4.cpp
 *
 * Author:      yut23
 * Created:     2022-12-05
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <sstream>
#include <string>

namespace aoc::day4 {

struct Assignment {
    int start{-1};
    int end{-1};
};

std::istream &operator>>(std::istream &is, Assignment &a) {
    is >> a.start >> a.end;
    return is;
}

struct Pair {
    Assignment first{};
    Assignment second{};

    bool includes() {
        return (first.start <= second.start && first.end >= second.end) ||
               (second.start <= first.start && second.end >= first.end);
    }

    bool overlaps() {
        return (first.start <= second.start && second.start <= first.end) ||
               (second.start <= first.start && first.start <= second.end);
    }
};

std::istream &operator>>(std::istream &is, Pair &p) {
    is >> p.first >> p.second;
    return is;
}

} // namespace aoc::day4

int main(int argc, char **argv) {
    auto infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string line;
    int include_count = 0, overlap_count = 0;
    while (std::getline(infile, line)) {
        using namespace aoc::day4;
        // simple and stupid: replace '-' and ',' with space
        std::ranges::replace_if(
            line, [](char c) { return c == '-' || c == ','; }, ' ');
        std::stringstream ss{line};
        // do stuff
        Pair pair{};
        ss >> pair;

        if (pair.includes()) {
            ++include_count;
        }
        if (pair.overlaps()) {
            ++overlap_count;
        }
    }
    std::cout << include_count << std::endl;
    std::cout << overlap_count << std::endl;
    return EXIT_SUCCESS;
}
