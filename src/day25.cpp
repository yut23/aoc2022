/******************************************************************************
 * File:        day25.cpp
 *
 * Author:      yut23
 * Created:     2022-12-26
 *****************************************************************************/

#include "lib.h"
#include <cassert>  // for assert
#include <iostream> // for cout
#include <map>      // for map
#include <string>   // for string, getline
#include <vector>   // for vector

namespace aoc::day25 {

long snafu_to_decimal(const std::string &snafu) {
    long value = 0;
    long place_value = 1;
    for (auto it = snafu.rbegin(); it != snafu.rend(); ++it) {
        int digit_value = 0;
        switch (*it) {
        case '2':
            digit_value = 2;
            break;
        case '1':
            digit_value = 1;
            break;
        case '0':
            digit_value = 0;
            break;
        case '-':
            digit_value = -1;
            break;
        case '=':
            digit_value = -2;
            break;
        default:
            assert(false);
            break;
        }
        value += place_value * digit_value;
        place_value *= 5;
    }
    return value;
}

std::string decimal_to_snafu(long value) {
    // convert to base-5
    std::string snafu_rev{};
    while (value > 0) {
        int digit_value = value % 5;
        switch (digit_value) {
        case 0:
            snafu_rev.push_back('0');
            break;
        case 1:
            snafu_rev.push_back('1');
            break;
        case 2:
            snafu_rev.push_back('2');
            break;
        case 3:
            snafu_rev.push_back('=');
            value += 5;
            break;
        case 4:
            snafu_rev.push_back('-');
            value += 5;
            break;
        default:
            std::cerr << "got illegal digit_value in int_to_snafu: "
                      << digit_value << "\n";
            assert(false);
        }
        value /= 5;
    }

    // reverse the string to get the right order
    return std::string{snafu_rev.rbegin(), snafu_rev.rend()};
}

void test_cases() {
    std::map<long, std::string> values = {
        {1, "1"},         {2, "2"},           {3, "1="},
        {4, "1-"},        {5, "10"},          {6, "11"},
        {7, "12"},        {8, "2="},          {9, "2-"},
        {10, "20"},       {15, "1=0"},        {20, "1-0"},
        {2022, "1=11-2"}, {12345, "1-0---0"}, {314159265, "1121-1110-1=0"},
    };

    for (const auto &[decimal_value, snafu_value] : values) {
        std::cerr << "checking " << decimal_value << " == " << snafu_value
                  << ":\n";
        long to_decimal = snafu_to_decimal(snafu_value);
        std::cerr << "  snafu_to_decimal(" << decimal_value << ") gives "
                  << to_decimal << "\n";
        std::string to_snafu = decimal_to_snafu(decimal_value);
        std::cerr << "  decimal_to_snafu(" << snafu_value << ") gives "
                  << to_snafu << "\n";
        assert(decimal_value == to_decimal);
        assert(snafu_value == to_snafu);
    }
}

} // namespace aoc::day25

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day25;
    if constexpr (aoc::DEBUG) {
        test_cases();
    }

    long sum = 0;
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        long value = snafu_to_decimal(line);
        assert(line == decimal_to_snafu(value));
        sum += value;
    }
    std::cout << decimal_to_snafu(sum) << std::endl;
    return 0;
}
