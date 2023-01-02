/******************************************************************************
 * File:        day17.cpp
 *
 * Author:      yut23
 * Created:     2023-01-02
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for transform, for_each, max
#include <array>     // for array
#include <iostream>  // for cout
#include <iterator>  // for back_inserter
#include <string>    // for string
#include <vector>    // for vector

namespace aoc::day17 {

// uses 9 bits in total, with the LSB and MSB used for wall collision detection
using line_t = unsigned short;

// these shapes have the lowest part at the front (only affects the L shape)
const std::array<std::vector<line_t>, 5> ROCK_SHAPES{{
    {
        0b0'0011110'0,
    },
    {
        0b0'0001000'0,
        0b0'0011100'0,
        0b0'0001000'0,
    },
    {
        0b0'0011100'0,
        0b0'0000100'0,
        0b0'0000100'0,
    },
    {
        0b0'0010000'0,
        0b0'0010000'0,
        0b0'0010000'0,
        0b0'0010000'0,
    },
    {
        0b0'0011000'0,
        0b0'0011000'0,
    },
}};

constexpr line_t WALL_MASK = 0b1'0000000'1;

class Board {
    std::vector<line_t> lines{};
    std::vector<Direction> jet_directions{};

    int rock_height = 0;
    std::vector<line_t> rock_shape{};

    int jet_index = 0;
    int rock_index = 0;

    void ensure_enough_lines(unsigned int new_height);

    bool will_collide(int rock_height) const;
    // returns true if the rock moved down successfully
    bool move_rock_down();
    void push_rock(bool debug);

  public:
    explicit Board(const std::string &);

    int height() const;
    void drop_rock(bool print, bool debug);

    friend std::ostream &operator<<(std::ostream &, const Board &);
};

Board::Board(const std::string &jets) {
    std::ranges::transform(
        jets, std::back_inserter(jet_directions),
        [](char c) { return c == '<' ? Direction::left : Direction::right; });
}

int Board::height() const {
    int h = lines.size();
    for (auto it = lines.rbegin(); it != lines.rend(); ++it, --h) {
        if (*it & ~WALL_MASK) {
            // there are non-wall cells
            break;
        }
    }
    return h;
}

void Board::ensure_enough_lines(unsigned int new_height) {
    if (lines.size() < new_height) {
        lines.resize(new_height, WALL_MASK);
    }
}

bool Board::will_collide(int rock_height) const {
    if (rock_height < 0) {
        return true;
    }
    auto line_it = lines.begin() + rock_height;
    for (auto rock_it = rock_shape.cbegin(); rock_it != rock_shape.cend();
         ++rock_it, ++line_it) {
        if (*line_it & *rock_it) {
            return true;
        }
    }
    return false;
}

void Board::push_rock(bool debug) {
    Direction shift_dir = jet_directions[jet_index];
    jet_index = (jet_index + 1) % jet_directions.size();
    if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << "Jet of gas pushes rock " << shift_dir;
        }
    }
    std::vector<line_t> prev_shape = rock_shape;
    // move by shift_amount
    std::ranges::for_each(rock_shape, [&shift_dir](line_t &line) {
        if (shift_dir == Direction::left) {
            line <<= 1;
        } else {
            line >>= 1;
        }
    });
    if (will_collide(rock_height)) {
        // undo the shift
        std::swap(rock_shape, prev_shape);
        if constexpr (aoc::DEBUG) {
            if (debug) {
                std::cerr << ", but nothing happens";
            }
        }
    }
    if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << ":\n" << *this << "\n";
        }
    }
}

bool Board::move_rock_down() {
    if (will_collide(rock_height - 1)) {
        return false;
    }
    // move rock down
    --rock_height;
    return true;
}

void Board::drop_rock(bool print, bool debug) {
    rock_height = height() + 3;
    rock_shape = ROCK_SHAPES[rock_index];
    rock_index = (rock_index + 1) % ROCK_SHAPES.size();
    ensure_enough_lines(rock_height + rock_shape.size());

    if constexpr (aoc::DEBUG) {
        if (print) {
            std::cerr << *this << "\n";
        }
    }

    // main tick loop
    while (true) {
        // push with jet
        push_rock(debug);
        if constexpr (aoc::DEBUG) {
            if (debug) {
                std::cerr << "Rock falls 1 unit";
            }
        }
        if (!move_rock_down()) {
            break;
        }
        if constexpr (aoc::DEBUG) {
            if (debug) {
                std::cerr << ":\n" << *this << "\n";
            }
        }
    }

    // add rock to stationary lines
    auto line_it = lines.begin() + rock_height;
    for (auto rock_it = rock_shape.cbegin(); rock_it != rock_shape.cend();
         ++rock_it, ++line_it) {
        *line_it |= *rock_it;
    }
    rock_height = 0;
    rock_shape.clear();

    if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << ", causing it to come to rest:\n" << *this << "\n";
        }
    }
}

std::ostream &operator<<(std::ostream &os, const Board &board) {
    int board_height = std::max(
        {board.height(),
         board.rock_height + static_cast<int>(board.rock_shape.size()), 1});
    int i;
    for (i = board_height - 1; i >= 0 && i >= board_height - 20; --i) {
        assert(i < static_cast<int>(board.lines.size()));
        int rock_idx = i - board.rock_height;
        os << '|';
        for (int mask = 1 << 7; mask != 1; mask >>= 1) {
            if (rock_idx >= 0 &&
                rock_idx < static_cast<int>(board.rock_shape.size()) &&
                board.rock_shape[rock_idx] & mask) {
                os << '@';
            } else if (board.lines[i] & mask) {
                os << '#';
            } else {
                os << '.';
            }
        }
        os << "|\n";
    }
    if (i == -1) {
        os << "+-------+\n";
    }
    return os;
}

} // namespace aoc::day17

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    std::string jets;
    infile >> jets;

    aoc::day17::Board board{jets};
    for (int i = 0; i < 2022; ++i) {
        if constexpr (aoc::DEBUG) {
            if (i < 11) {
                std::cerr << "Rock " << i + 1 << " begins falling:\n";
            }
        }
        board.drop_rock(i < 11, i < 3);
    }

    if constexpr (aoc::DEBUG) {
        std::cerr << "Final board:\n" << board << "\n";
    }
    std::cout << board.height() << "\n";

    return 0;
}
