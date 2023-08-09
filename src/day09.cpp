/******************************************************************************
 * File:        day09.cpp
 *
 * Author:      yut23
 * Created:     2022-12-09
 *****************************************************************************/

#include "lib.h"    // for Pos, Delta, Direction
#include <array>    // for array
#include <cassert>  // for assert
#include <cstdlib>  // for abs
#include <iostream> // for cout, cerr
#include <iterator> // for begin, end, cbegin
#include <set>      // for set
#include <string>   // for string
#include <vector>   // for vector

namespace aoc::day9 {

using Pos = aoc::Pos;
using Delta = aoc::Delta;
using Direction = aoc::Direction;

void move_tail(const Pos &head, Pos &tail) {
    Delta delta = head - tail;
    // tail + delta == head
    if (delta.chebyshev_distance() <= 1) {
        return;
    }
    if (std::abs(delta.dx) == 2) {
        delta.dx /= 2;
    }
    if (std::abs(delta.dy) == 2) {
        delta.dy /= 2;
    }
    tail += delta;
    assert((head - tail).chebyshev_distance() == 1);
}

template <typename T>
void move_rope(const Delta &delta, T &rope) {
    *std::begin(rope) += delta;
    auto first = std::cbegin(rope);
    auto second = std::begin(rope) + 1;
    for (; second != std::end(rope); ++first, ++second) {
        move_tail(*first, *second);
    }
}

template <typename T>
void print_rope(std::ostream &os, const T &rope, int width, int height) {
    std::vector<std::string> board(height, std::string(width, '.'));
    int i = rope.size() - 1;
    for (auto it = rope.crbegin(); it != rope.crend(); ++it, --i) {
        char c = std::to_string(i)[0];
        if (i == 0) {
            c = 'H';
        }
        if (it->x >= 0 && it->x < width && it->y >= 0 && it->y < height) {
            board[it->y][it->x] = c;
        }
    }
    for (auto it = board.crbegin(); it != board.crend(); ++it) {
        os << *it << std::endl;
    }
}

} // namespace aoc::day9

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day9;
    std::array<Pos, 10> rope{};
    std::set<Pos> second_knot_positions{rope[1]};
    std::set<Pos> tail_positions{rope.back()};

    Direction dir;
    int count;
    while (infile >> dir >> count) {
        Delta delta{dir};
        if constexpr (aoc::DEBUG) {
            std::cerr << "== " << dir << " " << count << " ==" << std::endl
                      << std::endl;
        }
        for (; count > 0; --count) {
            move_rope(delta, rope);
            second_knot_positions.insert(rope[1]);
            tail_positions.insert(rope.back());
            if constexpr (aoc::DEBUG) {
                print_rope(std::cerr, rope, 6, 5);
                std::cerr << std::endl;
            }
        }
    }
    std::cout << second_knot_positions.size() << std::endl;
    std::cout << tail_positions.size() << std::endl;
    return 0;
}
