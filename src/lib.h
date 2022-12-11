/******************************************************************************
 * File:        lib.h
 *
 * Author:      yut23
 * Created:     2022-12-01
 * Description: Common functions used in Advent of Code.
 *****************************************************************************/

#ifndef LIB_H_AT4RFPRV
#define LIB_H_AT4RFPRV

#include <cassert>  // for assert
#include <cstdlib>  // for exit
#include <fstream>  // for ifstream
#include <iostream> // for cout

namespace aoc {

#ifdef DEBUG_MODE
[[maybe_unused]] constexpr bool DEBUG = true;
#else
[[maybe_unused]] constexpr bool DEBUG = false;
#endif

template <class T>
struct SkipInputHelper {
    int count;
    friend std::istream &operator>>(std::istream &is, SkipInputHelper<T> s) {
        T temp;
        for (int i = 0; i < s.count && (is >> temp); ++i)
            ;
        return is;
    }
};

template <class T = std::string>
SkipInputHelper<T> skip(int count = 1) {
    return SkipInputHelper<T>{count};
}

/**
 * @brief  Parse command line arguments.
 * @return An istream for the specified input file.
 */
std::ifstream parse_args(int argc, char **argv) {
    if (argc != 2) {
        assert(argc >= 1);
        std::cout << "Usage: " << argv[0] << " <input file path>" << std::endl;
        std::exit(1);
    }
    return std::ifstream{argv[1]};
}

} // namespace aoc

#endif /* end of include guard: LIB_H_AT4RFPRV */
