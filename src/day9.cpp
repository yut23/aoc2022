/******************************************************************************
 * File:        day9.cpp
 *
 * Author:      yut23
 * Created:     2022-12-09
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for max
#include <cassert>   // for assert
#include <compare>   // for strong_ordering
#include <cstdlib>   // for abs
#include <iostream>  // for cout, cerr
#include <set>       // for set

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
        assert(std::abs(delta.dy) <= 1);
        delta.dx /= 2;
    } else if (std::abs(delta.dy) == 2) {
        assert(std::abs(delta.dx) <= 1);
        delta.dy /= 2;
    } else {
        std::cerr << "bad delta: " << delta << std::endl;
        assert(false);
    }
    tail += delta;
    assert(delta.chebyshev_distance() == 1);
}

} // namespace aoc::day9

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day9;
    Pos head{0, 0};
    Pos tail{0, 0};
    std::set<Pos> tail_positions{};
    tail_positions.insert(tail);

    Direction dir;
    int count;
    while (infile >> dir >> count) {
        Delta delta{dir};
        for (; count > 0; --count) {
            head += delta;
            move_tail(head, tail);
            tail_positions.insert(tail);
        }
    }
    std::cout << tail_positions.size() << std::endl;
    return 0;
}
