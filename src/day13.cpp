/******************************************************************************
 * File:        day13.cpp
 *
 * Author:      yut23
 * Created:     2022-12-14
 *****************************************************************************/

#include "lib.h"
#include <algorithm>        // for sort, upper_bound
#include <cassert>          // for assert
#include <compare>          // for strong_ordering
#include <cstddef>          // for size_t
#include <initializer_list> // for initializer_list
#include <iostream>         // for cout
#include <iterator>         // for distance
#include <utility>          // for move
#include <vector>           // for vector

namespace aoc::day13 {

struct Packet {
  private:
    bool _is_list{false};
    bool _is_int{false};
    void check() const { assert(!(_is_list && _is_int)); }

  public:
    int value{-1};
    std::vector<Packet> contents{};

    Packet() = default;
    explicit Packet(int value) : _is_list(false), _is_int(true), value(value) {}
    explicit Packet(std::initializer_list<Packet> init)
        : _is_list(true), _is_int(false), contents(init) {}

    bool is_list() const {
        check();
        return _is_list;
    }
    bool is_int() const {
        check();
        return _is_int;
    }
    bool is_either() const {
        check();
        return _is_list || _is_int;
    }

    bool operator==(const Packet &rhs) const = default;
    friend std::istream &operator>>(std::istream &, Packet &);
};

std::istream &operator>>(std::istream &is, Packet &packet) {
    // skip whitespace
    is >> std::ws;
    char c = is.peek();
    if (is.eof()) {
        return is;
    }
    // reset the packet
    packet = Packet{};
    if (c == '[') {
        packet._is_list = true;
        // read list
        while ((c = is.get()) != ']') {
            assert(c == '[' || c == ',');
            if (is.peek() == ']') {
                // empty list
                continue;
            }
            Packet p;
            is >> p;
            assert(p.is_either());
            packet.contents.push_back(p);
        }
        assert(c == ']');
    } else {
        packet._is_int = true;
        // read integer
        assert(c >= '0' && c <= '9');
        is >> packet.value;
    }
    return is;
}

std::ostream &operator<<(std::ostream &os, const Packet &packet) {
    if (packet.is_list()) {
        os << '[';
        bool first = true;
        for (const auto &p : packet.contents) {
            if (!first) {
                os << ',';
            }
            os << p;
            first = false;
        }
        os << ']';
    } else if (packet.is_int()) {
        os << packet.value;
    } else {
        os << "Packet()";
    }
    return os;
}

std::weak_ordering operator<=>(const Packet &lhs, const Packet &rhs) {
    assert(lhs.is_either() && rhs.is_either());
    if (lhs.is_int() && rhs.is_int()) {
        // both integers, compare values
        return lhs.value <=> rhs.value;
    }
    if (lhs.is_list() && rhs.is_list()) {
        // both lists, compare recursively
        for (std::size_t i = 0;
             i < lhs.contents.size() && i < rhs.contents.size(); ++i) {
            std::weak_ordering cmp = lhs.contents[i] <=> rhs.contents[i];
            if (cmp != std::weak_ordering::equivalent) {
                return cmp;
            }
        }
        // compare the lengths if all the contained packets are equal up to the
        // end of the shorter list
        return lhs.contents.size() <=> rhs.contents.size();
    }
    if (lhs.is_int() && rhs.is_list()) {
        // convert lhs to a list
        return Packet({lhs}) <=> rhs;
    }
    if (lhs.is_list() && rhs.is_int()) {
        // convert rhs to a list
        return lhs <=> Packet({rhs});
    }
    assert(false);
}

} // namespace aoc::day13

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day13;
    Packet left, right;
    std::vector<Packet> packets;
    int result = 0;
    for (int i = 1; infile >> left >> right; ++i) {
        if constexpr (aoc::DEBUG) {
            std::cerr << "left: " << left << "\nright: " << right << std::endl;
        }
        if (left < right) {
            if constexpr (aoc::DEBUG) {
                std::cerr << i
                          << ": left < right, so inputs are in right order\n\n";
            }
            result += i;
        } else if constexpr (aoc::DEBUG) {
            std::cerr
                << i
                << ": left >= right, so inputs are not in the right order\n\n";
        }
        packets.push_back(std::move(left));
        packets.push_back(std::move(right));
    }
    std::cout << result << std::endl;

    // add divider packets
    Packet divider_start({Packet({Packet(2)})});
    Packet divider_end({Packet({Packet(6)})});
    packets.push_back(divider_start);
    packets.push_back(divider_end);

    std::ranges::sort(packets);
    // use upper_bound to get the index of the packet after the divider
    int start_idx = std::distance(
        packets.begin(), std::ranges::upper_bound(packets, divider_start));
    int end_idx = std::distance(packets.begin(),
                                std::ranges::upper_bound(packets, divider_end));
    std::cout << start_idx * end_idx << std::endl;
    return 0;
}
