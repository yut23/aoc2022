/******************************************************************************
 * File:        day23.cpp
 *
 * Author:      yut23
 * Created:     2022-12-23
 *****************************************************************************/

#include "lib.h"
#include <algorithm>  // for any_of, all_of, for_each, count_if
#include <cassert>    // for assert
#include <cstdlib>    // for abs
#include <deque>      // for deque
#include <functional> // for plus
#include <iostream>   // for cout
#include <list>       // for list
#include <numeric>    // for transform_reduce
#include <string>     // for string, getline

namespace aoc::day23 {

enum class MoveDirection { north, east, south, west };
std::ostream &operator<<(std::ostream &os, const MoveDirection &dir) {
    switch (dir) {
    case MoveDirection::north:
        os << "north";
        break;
    case MoveDirection::east:
        os << "east";
        break;
    case MoveDirection::south:
        os << "south";
        break;
    case MoveDirection::west:
        os << "west";
        break;
    }
    return os;
}

struct Cell {
    bool is_elf = false;
    bool conflict = false;
    Cell *move_from = nullptr;

    void reset_proposal_state() {
        move_from = nullptr;
        conflict = false;
    }
};

class Grid {
  private:
    bool initialized = false;
    int x_lo, y_lo;
    int x_hi, y_hi;
    std::list<MoveDirection> proposal_order = {
        MoveDirection::north,
        MoveDirection::south,
        MoveDirection::west,
        MoveDirection::east,
    };

    int elf_count = 0;

    std::deque<std::deque<Cell>> grid;

    // adds abs(signed_count) rows to the north if negative or south if positive
    void add_rows(int signed_count);
    // adds abs(signed_count) columns to the west if negative or east if
    // positive
    void add_cols(int signed_count);

    bool in_bounds(int x, int y) const;
    // get a reference to a specific cell, expanding the grid size if necessary
    Cell &get(int x, int y);
    const Cell &cget(int x, int y) const {
        assert(in_bounds(x, y));
        return grid[y - y_lo][x - x_lo];
    }
    bool is_empty(int x, int y) const;

    bool is_move_valid(int x, int y, MoveDirection dir, Cell *&dest);
    bool propose_move(int x, int y);

    // Reduce the area by removing outside edges with no elves (only removes 1
    // from all sides). Returns true if any edges were removed.
    bool contract();

    friend std::ostream &operator<<(std::ostream &, const Grid &);

  public:
    void check_invariants(bool skip_cols = false) const;
    void add_line(const std::string &line);
    bool propose_moves();
    void make_moves();
    int count_empty() const;
};

inline bool Grid::in_bounds(int x, int y) const {
    return x >= x_lo && x < x_hi && y >= y_lo && y < y_hi;
}
inline bool Grid::is_empty(int x, int y) const {
    return !in_bounds(x, y) || !cget(x, y).is_elf;
}

void Grid::check_invariants(bool skip_cols) const {
    if (!initialized) {
        return;
    }
    // make sure the row lengths match
    std::deque<Cell>::size_type size = grid[0].size();
    assert(std::ranges::all_of(
        grid, [=](const std::deque<Cell> &row) { return row.size() == size; }));
    // make sure the number of elves is the same
    int curr_elf_count = std::transform_reduce(
        grid.cbegin(), grid.cend(), 0, std::plus{}, [](const auto &row) {
            return std::ranges::count_if(
                row, [](const auto &cell) { return cell.is_elf; });
        });
    if (elf_count != curr_elf_count) {
        std::cerr << "expected elf count to be " << elf_count << ", but got "
                  << curr_elf_count << "\n";
        assert(elf_count == curr_elf_count);
    }
    // make sure there's at least one non-empty cell on the outside edges
    auto nonempty_cell = [](const Cell &value) { return value.is_elf; };
    // check north edge
    assert(std::ranges::any_of(grid.front(), nonempty_cell));
    // check south edge
    assert(std::ranges::any_of(grid.back(), nonempty_cell));
    if (!skip_cols) {
        // check west edge
        assert(std::ranges::any_of(
            grid, [](const auto &row) { return row.front().is_elf; }));
        // check east edge
        assert(std::ranges::any_of(
            grid, [](const auto &row) { return row.back().is_elf; }));
    }
}

bool Grid::contract() {
    assert(initialized);
    bool did_remove;
    // make sure there's at least one non-empty cell on the outside edges
    auto empty_cell = [](const Cell &value) { return !value.is_elf; };
    if (std::ranges::all_of(grid.front(), empty_cell)) {
        // remove north edge
        ++y_lo;
        grid.pop_front();
        did_remove = true;
    }
    if (std::ranges::all_of(grid.back(), empty_cell)) {
        // remove south edge
        --y_hi;
        grid.pop_back();
        did_remove = true;
    }
    if (std::ranges::all_of(
            grid, [](const auto &row) { return !row.front().is_elf; })) {
        // remove west edge
        ++x_lo;
        std::ranges::for_each(grid, [](auto &row) { row.pop_front(); });
        did_remove = true;
    }
    if (std::ranges::all_of(
            grid, [](const auto &row) { return !row.back().is_elf; })) {
        // remove east edge
        --x_hi;
        std::ranges::for_each(grid, [](auto &row) { row.pop_back(); });
        did_remove = true;
    }
    return did_remove;
}

Cell &Grid::get(int x, int y) {
    if (!initialized) {
        x_lo = x_hi = x;
        y_lo = y_hi = y;
        initialized = true;
    }
    if (!in_bounds(x, y)) {
        if (y < y_lo) {
            add_rows(y - y_lo);
        } else if (y >= y_hi) {
            add_rows(y - y_hi + 1);
        }
        if (x < x_lo) {
            add_cols(x - x_lo);
        } else if (x >= x_hi) {
            add_cols(x - x_hi + 1);
        }
    }
    return grid[y - y_lo][x - x_lo];
}

void Grid::add_rows(int signed_count) {
    assert(initialized);
    assert(signed_count != 0);
    int count = std::abs(signed_count);
    auto dest_it = grid.cbegin();
    if (signed_count < 0) {
        // add to front
        dest_it = grid.cbegin();
        y_lo -= count;
    } else if (signed_count > 0) {
        // add to back
        dest_it = grid.cend();
        y_hi += count;
    }
    grid.insert(dest_it, count, std::deque<Cell>(x_hi - x_lo));
}

void Grid::add_cols(int signed_count) {
    assert(initialized);
    assert(signed_count != 0);
    int count = std::abs(signed_count);
    if (signed_count < 0) {
        // add to front
        std::ranges::for_each(
            grid, [&count](auto &row) { row.insert(row.cbegin(), count, {}); });
        x_lo -= count;
    } else if (signed_count > 0) {
        // add to back
        std::ranges::for_each(
            grid, [&count](auto &row) { row.insert(row.cend(), count, {}); });
        x_hi += count;
    }
}

void Grid::add_line(const std::string &line) {
    check_invariants(true);
    int y = initialized ? y_hi : 0;
    for (int x = 0; char c : line) {
        if (c == '#') {
            get(x, y).is_elf = true;
            ++elf_count;
        }
        ++x;
    }
}

bool Grid::is_move_valid(int x, int y, MoveDirection dir, Cell *&dest) {
    switch (dir) {
    case MoveDirection::north:
        if (is_empty(x - 1, y - 1) && is_empty(x, y - 1) &&
            is_empty(x + 1, y - 1)) {
            dest = &get(x, y - 1);
            return true;
        }
        return false;
    case MoveDirection::south:
        if (is_empty(x - 1, y + 1) && is_empty(x, y + 1) &&
            is_empty(x + 1, y + 1)) {
            dest = &get(x, y + 1);
            return true;
        }
        return false;
    case MoveDirection::west:
        if (is_empty(x - 1, y - 1) && is_empty(x - 1, y) &&
            is_empty(x - 1, y + 1)) {
            dest = &get(x - 1, y);
            return true;
        }
        return false;
    case MoveDirection::east:
        if (is_empty(x + 1, y - 1) && is_empty(x + 1, y) &&
            is_empty(x + 1, y + 1)) {
            dest = &get(x + 1, y);
            return true;
        }
        return false;
    }
    assert(false);
}

bool Grid::propose_move(int x, int y) {
    assert(in_bounds(x, y));
    Cell &cell = get(x, y);
    if (!cell.is_elf) {
        return false;
    }
    // do nothing if all neighbors are empty
    bool found_neighbors = false;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0)
                continue;
            if (!is_empty(x + dx, y + dy)) {
                found_neighbors = true;
                break;
            }
        }
    }
    if (!found_neighbors) {
        return false;
    }
    for (MoveDirection direction : proposal_order) {
        if constexpr (aoc::DEBUG) {
            std::cerr << "checking " << direction << " for " << x << ", " << y
                      << ": ";
        }
        Cell *dest = nullptr;
        if (!is_move_valid(x, y, direction, dest)) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "blocked\n";
            }
            continue;
        }
        assert(dest != nullptr);
        if (dest->move_from != nullptr || dest->conflict) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "conflict\n";
            }
            dest->conflict = true;
            dest->move_from = nullptr;
        } else {
            if constexpr (aoc::DEBUG) {
                std::cerr << "success\n";
            }
            dest->move_from = &cell;
        }
        break;
    }
    return true;
}

bool Grid::propose_moves() {
    check_invariants();
    bool did_anything = false;
    for (int y = y_lo; y < y_hi; ++y) {
        for (int x = x_lo; x < x_hi; ++x) {
            did_anything |= propose_move(x, y);
        }
    }
    return did_anything;
}

void Grid::make_moves() {
    for (int y = y_lo; y < y_hi; ++y) {
        for (int x = x_lo; x < x_hi; ++x) {
            Cell &dest = get(x, y);
            if (dest.move_from != nullptr && !dest.conflict) {
                dest.move_from->is_elf = false;
                dest.is_elf = true;
            }
            dest.reset_proposal_state();
        }
    }
    contract();
    // update proposal order: move first element to the end
    proposal_order.splice(proposal_order.cend(), proposal_order,
                          proposal_order.cbegin());
}

int Grid::count_empty() const {
    return (x_hi - x_lo) * (y_hi - y_lo) - elf_count;
}

std::ostream &operator<<(std::ostream &os, const Grid &grid) {
    for (const auto &row : grid.grid) {
        for (const auto &cell : row) {
            os << (cell.is_elf ? '#' : '.');
        }
        os << "\n";
    }
    return os;
}

} // namespace aoc::day23

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day23;
    Grid grid;
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        grid.add_line(line);
    }

    if constexpr (aoc::DEBUG) {
        std::cerr << "== Initial State ==\n" << grid << "\n";
    }
    for (int round = 1; round <= 10; ++round) {
        if (!grid.propose_moves()) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "Done\n";
            }
            break;
        }
        grid.make_moves();
        if constexpr (aoc::DEBUG) {
            std::cerr << "== End of Round " << round << " ==\n" << grid << "\n";
        }
        grid.check_invariants();
    }
    std::cout << grid.count_empty() << std::endl;
    return 0;
}
