/******************************************************************************
 * File:        day17.cpp
 *
 * Author:      yut23
 * Created:     2023-01-02
 *****************************************************************************/

#include "lib.h"
#include <algorithm>   // for transform, for_each, max
#include <array>       // for array
#include <bit>         // for bit_width
#include <iostream>    // for cout, cerr
#include <iterator>    // for back_inserter, distance
#include <map>         // for map
#include <string>      // for string
#include <type_traits> // for remove_const_t
#include <vector>      // for vector

namespace aoc::day17 {

// uses 7 bits for each line
using line_t = unsigned char;
using rock_shape_t = std::array<line_t, 4>;

// these shapes have the lowest part at the front (only affects the L shape)
const std::array<rock_shape_t, 5> ROCK_SHAPES{{
    {
        0b0011110,
        0b0000000,
        0b0000000,
        0b0000000,
    },
    {
        0b0001000,
        0b0011100,
        0b0001000,
        0b0000000,
    },
    {
        0b0011100,
        0b0000100,
        0b0000100,
        0b0000000,
    },
    {
        0b0010000,
        0b0010000,
        0b0010000,
        0b0010000,
    },
    {
        0b0011000,
        0b0011000,
        0b0000000,
        0b0000000,
    },
}};
constexpr std::array<int, 5> ROCK_HEIGHTS{{1, 3, 3, 4, 2}};

struct DropPositions {
    unsigned short raw;

    DropPositions() : raw(0) {
        // fill with invalid positions
        for (int i = 0; i < 5; ++i) {
            raw |= 7 << (i * 3);
        }
    }

    unsigned char get(int rock_index) {
        return static_cast<unsigned char>(raw >> (rock_index * 3)) & 0b111;
    }
    void set(int rock_index, unsigned char pos) {
        unsigned short mask = 0b111 << (rock_index * 3);
        raw = (raw & ~mask) | ((pos & 0b111) << (rock_index * 3));
    }
    void copy_from(const DropPositions &other, int rock_index) {
        unsigned short mask = 0b111 << (rock_index * 3);
        raw = (raw & ~mask) | (other.raw & mask);
    }
};

class Board {
    std::vector<line_t> lines{};
    std::vector<Direction> jet_directions{};
    long floor_height = 0;
    long internal_height = 0;

    long rock_number = 0;

    long rock_pos = 0;
    int rock_height = 0;
    rock_shape_t rock_shape{};

    int rock_index = 0;
    int jet_index = 0;

    struct LoopInfo {
        long rock_number;
        long height;
    };

    static constexpr unsigned int cache_size = 2;
    std::map<const std::array<unsigned short, cache_size>, LoopInfo>
        loop_cache{};
    std::array<DropPositions, cache_size> drop_pos_arr{};
    bool found_loop = false;
    bool at_loop_start = false;
    LoopInfo loop_start;
    LoopInfo loop_end;

    void ensure_enough_lines(long new_height);

    bool will_collide(const Direction &dir) const;
    // returns true if the rock moved down successfully
    bool move_rock_down();
    void push_rock(bool debug);

    void save_to_cache();
    void drop_rock();

  public:
    explicit Board(const std::string &);

    long height() const { return floor_height + internal_height; }
    void drop_until(long count);

    friend std::ostream &operator<<(std::ostream &, const Board &);
};

Board::Board(const std::string &jets) {
    std::ranges::transform(
        jets, std::back_inserter(jet_directions),
        [](char c) { return c == '<' ? Direction::left : Direction::right; });
    save_to_cache();
}

void Board::ensure_enough_lines(long new_height) {
    if (static_cast<long>(lines.size()) < new_height) {
        lines.resize(new_height, 0);
    }
}

bool Board::will_collide(const Direction &dir) const {
    auto line_it = lines.begin() + rock_pos;
    if (dir == Direction::down) {
        if (rock_pos == 0) {
            return true;
        }
        --line_it;
    }
    line_t rock_val;
    for (auto rock_it = rock_shape.cbegin(); rock_it != rock_shape.cend();
         ++rock_it, ++line_it) {
        if (dir == Direction::left) {
            if (*rock_it & (1 << 6)) {
                return true;
            }
            rock_val = *rock_it << 1;
        } else if (dir == Direction::right) {
            if (*rock_it & (1 << 0)) {
                return true;
            }
            rock_val = *rock_it >> 1;
        } else {
            rock_val = *rock_it;
        }
        if (*line_it & rock_val) {
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
    if (!will_collide(shift_dir)) {
        if (shift_dir == Direction::left) {
            std::ranges::for_each(rock_shape, [](line_t &line) { line <<= 1; });
        } else {
            std::ranges::for_each(rock_shape, [](line_t &line) { line >>= 1; });
        }
    } else if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << ", but nothing happens";
        }
    }
    if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << ":\n" << *this << "\n";
        }
    }
}

bool Board::move_rock_down() {
    if (will_collide(Direction::down)) {
        return false;
    }
    // move rock down
    --rock_pos;
    return true;
}

void Board::save_to_cache() {
    std::remove_const_t<typename decltype(loop_cache)::key_type> test_key{};
    std::ranges::transform(drop_pos_arr, test_key.begin(),
                           [](const auto &drop_pos) { return drop_pos.raw; });
    auto it = loop_cache.find(test_key);
    if (it != loop_cache.end()) {
        found_loop = true;
        loop_start = it->second;
        loop_end = LoopInfo{rock_number, height()};
        loop_cache.clear();
        if constexpr (aoc::DEBUG) {
            std::cerr << "found loop starting at rock number "
                      << loop_start.rock_number << " and floor height "
                      << loop_start.height << ", with length "
                      << loop_end.rock_number - loop_start.rock_number
                      << " and height " << loop_end.height - loop_start.height
                      << "\n";
        }
    } else {
        loop_cache[test_key] = LoopInfo{rock_number, height()};
    }
}

void Board::drop_rock() {
    ++rock_number;
    bool debug = rock_number < 3;
    rock_pos = internal_height + 3;
    rock_shape = ROCK_SHAPES[rock_index];
    rock_height = ROCK_HEIGHTS[rock_index];
    ensure_enough_lines(rock_pos + rock_shape.size());

    if constexpr (aoc::DEBUG) {
        if (rock_number < 11) {
            std::cerr << "Rock " << rock_number << " begins falling:\n"
                      << *this << "\n";
        }
    }

    // main update loop
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

    unsigned char drop_pos = 0;
    internal_height = std::max(internal_height, rock_pos + rock_height);
    // add rock to stationary lines
    auto line_it = lines.begin() + rock_pos;
    for (auto rock_it = std::begin(rock_shape); rock_it != std::end(rock_shape);
         ++rock_it, ++line_it) {
        *line_it |= *rock_it;
        drop_pos = std::max(drop_pos, std::bit_width(*rock_it));
    }
    for (unsigned int i = 0; i < drop_pos_arr.size() - 1; ++i) {
        drop_pos_arr[i].copy_from(drop_pos_arr[i + 1], rock_index);
    }
    drop_pos_arr.back().set(rock_index, drop_pos);
    rock_pos = 0;
    rock_height = 0;

    if constexpr (aoc::DEBUG) {
        if (debug) {
            std::cerr << ", causing it to come to rest:\n" << *this << "\n";
        }
    }

    if (found_loop) {
        at_loop_start = (rock_number - loop_start.rock_number) %
                            (loop_end.rock_number - loop_start.rock_number) ==
                        0;
    } else if (rock_index == 0) {
        save_to_cache();
    }

    rock_index = (rock_index + 1) % ROCK_SHAPES.size();
}

void Board::drop_until(long count) {
    while (rock_number < count && !(found_loop && at_loop_start)) {
        drop_rock();
    }
    if (rock_number == count) {
        return;
    }
    if constexpr (aoc::DEBUG) {
        std::cerr << "skipping forward...\n";
    }
    long loop_size = loop_end.rock_number - loop_start.rock_number;
    long loop_height = loop_end.height - loop_start.height;
    long iter_count = (count - rock_number) / loop_size;
    rock_number += loop_size * iter_count;
    floor_height += loop_height * iter_count;
    if constexpr (aoc::DEBUG) {
        std::cerr << "skipped to rock " << rock_number
                  << ", height=" << height() << "\n";
        std::cerr << "now continuing manually to rock " << count << "...\n";
    }
    while (rock_number < count) {
        drop_rock();
    }
}

std::ostream &operator<<(std::ostream &os, const Board &board) {
    long board_height = std::max(
        {board.internal_height, board.rock_pos + board.rock_height, 1L});
    long i;
    for (i = board_height - 1; i >= 0 && i >= board_height - 20; --i) {
        // assert(i < static_cast<long>(board.lines.size()));
        long rock_idx = i - board.rock_pos;
        os << '|';
        for (line_t mask = 1 << 6; mask != 0; mask >>= 1) {
            if (rock_idx >= 0 && rock_idx < board.rock_height &&
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
    if (i == -1 && board.floor_height == 0) {
        os << "+-------+\n";
    } else {
        os << "(lines 0-" << board.floor_height + i << ")\n";
    }
    return os;
}

} // namespace aoc::day17

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    std::string jets;
    infile >> jets;

    aoc::day17::Board board{jets};
    board.drop_until(2022);
    std::cout << board.height() << "\n";
    board.drop_until(1000000000000);
    if constexpr (aoc::DEBUG) {
        std::cerr << "Final board:\n" << board << "\n";
    }
    std::cout << board.height() << "\n";

    return 0;
}
