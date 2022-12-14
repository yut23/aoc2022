/******************************************************************************
 * File:        day14.cpp
 *
 * Author:      yut23
 * Created:     2022-12-14
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for minmax
#include <cassert>   // for assert
#include <iostream>  // for cout
#include <sstream>   // for istringstream
#include <string>    // for string, getline
#include <utility>   // for pair
#include <vector>    // for vector

namespace aoc::day14 {

constexpr int INITIAL_X = 500;
constexpr int INITIAL_Y = 0;

enum class CellType { air, stone, sand };
std::ostream &operator<<(std::ostream &os, CellType celltype) {
    switch (celltype) {
    case CellType::air:
        os << '.';
        break;
    case CellType::stone:
        os << '#';
        break;
    case CellType::sand:
        os << 'o';
        break;
    }
    return os;
}

class Grid {
    static constexpr int min_y = 0;
    int min_x{INITIAL_X}, max_x{INITIAL_X}, max_y{INITIAL_Y};

    std::vector<std::vector<CellType>> cells;

    constexpr std::pair<int, int> lookup_pos(int x, int y) const {
        return {y, x - min_x};
    }
    void add_line(int x1, int y1, int x2, int y2);
    void place_sand(int x, int y);

    // check if the given coordinates are air
    bool open(int x, int y) const;
    // returns true if there is nothing below the given point
    bool over_abyss(int x, int y) const;

    // moves a sand grain once, returns false if no move is possible
    bool single_tick(int &x, int &y) const;

  public:
    explicit Grid(const std::vector<std::vector<aoc::Pos>> &scan);

    // spawn a sand grain, move it until it settles, then add it to the grid
    // returns false if the grain falls into the abyss
    bool add_sand_grain();

    // add an "infinite" floor 2 spaces below max_y
    void setup_part_2();

    friend std::ostream &operator<<(std::ostream &, const Grid &);
};

Grid::Grid(const std::vector<std::vector<aoc::Pos>> &scan) {
    for (const auto &path : scan) {
        for (const auto &pos : path) {
            if (pos.x > max_x) {
                max_x = pos.x;
            }
            if (pos.x < min_x) {
                min_x = pos.x;
            }
            if (pos.y > max_y) {
                max_y = pos.y;
            }
        }
    }
    // a larger grid is needed for part 2
    max_x = std::max(INITIAL_X + (max_y + 2) + 1, max_x);
    min_x = std::min(INITIAL_X - (max_y + 2) - 1, min_x);
    cells.resize((max_y + 2) - min_y + 1,
                 std::vector<CellType>(max_x - min_x + 1, CellType::air));
    for (const auto &path : scan) {
        auto pos_1 = path.cbegin();
        auto pos_2 = pos_1 + 1;
        for (; pos_2 != path.cend(); ++pos_1, ++pos_2) {
            add_line(pos_1->x, pos_1->y, pos_2->x, pos_2->y);
        }
    }
}

void Grid::setup_part_2() {
    max_y += 2;
    add_line(min_x, max_y, max_x, max_y);
}

void Grid::add_line(int x1, int y1, int x2, int y2) {
    auto [r1, c1] = lookup_pos(x1, y1);
    auto [r2, c2] = lookup_pos(x2, y2);
    if (c1 != c2) {
        // horizontal
        assert(r1 == r2);
        auto [clo, chi] = std::minmax(c1, c2);
        for (int c = clo; c <= chi; ++c) {
            cells[r1][c] = CellType::stone;
        }
    } else if (r1 != r2) {
        // vertical
        assert(c1 == c2);
        auto [rlo, rhi] = std::minmax(r1, r2);
        for (int r = rlo; r <= rhi; ++r) {
            cells[r][c1] = CellType::stone;
        }
    } else {
        // path with length 1
        cells[r1][c1] = CellType::stone;
    }
}

void Grid::place_sand(int x, int y) {
    auto [row, col] = lookup_pos(x, y);
    std::vector<CellType>::reference value = cells[row][col];
    assert(value == CellType::air);
    value = CellType::sand;
}

bool Grid::open(int x, int y) const {
    if (over_abyss(x, y)) {
        return true;
    }
    auto [row, col] = lookup_pos(x, y);
    return cells[row][col] == CellType::air;
}

bool Grid::over_abyss(int x, int y) const {
    // very simple implementation, can be optimized later if needed
    return y > max_y || x < min_x || x > max_x;
}

std::ostream &operator<<(std::ostream &os, const Grid &grid) {
    for (const auto &row : grid.cells) {
        for (const auto &cell : row) {
            os << cell;
        }
        os << '\n';
    }
    return os;
}

bool Grid::single_tick(int &x, int &y) const {
    // try moving straight down
    if (open(x, y + 1)) {
        ++y;
        return true;
    }
    // try moving down and left
    if (open(x - 1, y + 1)) {
        --x;
        ++y;
        return true;
    }
    // try moving down and right
    if (open(x + 1, y + 1)) {
        ++x;
        ++y;
        return true;
    }
    return false;
}

bool Grid::add_sand_grain() {
    int x = INITIAL_X, y = INITIAL_Y;
    if (!open(x, y)) {
        return false;
    }
    while (single_tick(x, y)) {
        if (over_abyss(x, y)) {
            return false;
        }
    }
    place_sand(x, y);
    return true;
}

} // namespace aoc::day14

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day14;
    // read file line-by-line
    std::string line;
    std::vector<std::vector<aoc::Pos>> scan{};
    int x, y;
    while (std::getline(infile, line)) {
        std::istringstream ss{line};
        scan.emplace_back();
        while (ss >> x >> aoc::skip<char>() >> y) {
            scan.back().emplace_back(x, y);
            // skip "->"
            ss >> aoc::skip(1);
        }
    }
    Grid grid{scan};
    if constexpr (aoc::DEBUG) {
        std::cerr << grid << std::endl;
    }

    int i = 0;
    for (; grid.add_sand_grain(); ++i) {
        if constexpr (aoc::DEBUG) {
            std::cerr << grid << std::endl;
        }
    }
    std::cout << i << std::endl;

    grid.setup_part_2();
    for (; grid.add_sand_grain(); ++i)
        ;
    std::cout << i << std::endl;
    return 0;
}
