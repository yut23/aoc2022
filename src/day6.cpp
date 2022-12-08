/******************************************************************************
 * File:        day6.cpp
 *
 * Author:      yut23
 * Created:     2022-12-06
 *****************************************************************************/

#include "lib.h"
#include <cassert>
#include <string>
#include <string_view>
#include <unordered_set>

namespace aoc::day6 {

/// Returns the index of the first character after the marker occurrence.
int find_marker(std::string text, int window_size) {
    assert(text.length() >= window_size);
    std::unordered_multiset<char> lookup{};
    auto old_it = text.cbegin();
    auto new_it = text.cbegin();
    int count = 0;
    while (new_it != text.cend()) {
        assert(lookup.size() <= window_size);
        if (lookup.size() == window_size) {
            // remove oldest character from lookup
            lookup.erase(lookup.find(*old_it));
            // check if old character is still in lookup (we removed a
            // duplicate)
            if (lookup.contains(*old_it)) {
                --count;
            }
        }
        // check if new character is in lookup (we are adding a duplicate)
        if (lookup.contains(*new_it)) {
            ++count;
        }
        // add the new character to lookup
        lookup.insert(*new_it);
        if constexpr (aoc::DEBUG) {
            std::cerr << "old=" << std::distance(text.cbegin(), old_it) << ": "
                      << *old_it
                      << " new=" << std::distance(text.cbegin(), new_it) << ": "
                      << *new_it << " count=" << count
                      << " lookup=" << std::string_view{old_it, new_it}
                      << " size=" << lookup.size() << std::endl;
        }
        if (new_it - old_it >= window_size) {
            ++old_it;
        }
        ++new_it;
        if (lookup.size() == window_size && count == 0) {
            // found a sequence with no duplicates
            break;
        }
    }
    return std::distance(text.cbegin(), new_it);
}

} // namespace aoc::day6

int main(int argc, char **argv) {
    auto infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    std::string line;
    infile >> line;
    std::cout << aoc::day6::find_marker(line, 4) << std::endl;
    std::cout << aoc::day6::find_marker(line, 14) << std::endl;
    return EXIT_SUCCESS;
}
