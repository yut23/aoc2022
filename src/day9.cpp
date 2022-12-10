/******************************************************************************
 * File:        day9.cpp
 *
 * Author:      yut23
 * Created:     2022-12-09
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for max
#include <array>     // for array
#include <cassert>   // for assert
#include <compare>   // for strong_ordering
#include <cstdlib>   // for abs
#include <iostream>  // for cout, cerr
#include <iterator>  // for begin, end, cbegin
#include <set>       // for set
#include <string>    // for string
#include <vector>    // for vector

namespace aoc::day9 {

enum Direction : char { up = 'U', down = 'D', left = 'L', right = 'R' };
std::istream &operator>>(std::istream &is, enum Direction &dir) {
    char ch = 0;
    if (is >> ch) {
        dir = static_cast<Direction>(ch);
    }
    return is;
}

struct Delta {
    int dx;
    int dy;

    Delta(int dx, int dy) : dx(dx), dy(dy) {}
    explicit Delta(Direction dir);

    int chebyshev_distance() const {
        return std::max(std::abs(dx), std::abs(dy));
    }
};

Delta::Delta(Direction dir) : dx(0), dy(0) {
    switch (dir) {
    case Direction::up:
        dy = 1;
        break;
    case Direction::down:
        dy = -1;
        break;
    case Direction::right:
        dx = 1;
        break;
    case Direction::left:
        dx = -1;
        break;
    }
}

std::ostream &operator<<(std::ostream &os, const Delta &delta) {
    os << "Delta(" << delta.dx << ", " << delta.dy << ")";
    return os;
}

struct Pos {
    int x;
    int y;

    Pos() : x(0), y(0) {}
    Pos(int x, int y) : x(x), y(y) {}

    // we can add a Delta to a Pos, but not another Pos
    Pos &operator+=(const Delta &rhs) {
        x += rhs.dx;
        y += rhs.dy;
        return *this;
    }

    std::strong_ordering operator<=>(const Pos &) const = default;
};
// this takes lhs by copy, so it doesn't modify the original lhs
inline Pos operator+(Pos lhs, const Delta &rhs) {
    lhs += rhs;
    return lhs;
}
// can subtract two Pos, yielding a Delta
inline Delta operator-(const Pos &lhs, const Pos &rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

std::ostream &operator<<(std::ostream &os, const Pos &pos) {
    os << "Pos(" << pos.x << ", " << pos.y << ")";
    return os;
}

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

template <typename T> void move_rope(const Delta &delta, T &rope) {
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
