/******************************************************************************
 * File:        day18.cpp
 *
 * Author:      yut23
 * Created:     2022-12-18
 *****************************************************************************/

#include "lib.h"
#include <cassert>    // for assert
#include <functional> // for function
#include <iostream>   // for cout
#include <queue>      // for queue
#include <sstream>    // for istringstream
#include <string>     // for string, getline
#include <vector>     // for vector

namespace aoc::day18 {

struct Cell {
    bool lava : 1 = false;
    bool exterior : 1 = false;
    bool flood_visited : 1 = false;
};
static_assert(sizeof(Cell) == 1, "Cell not packed");

class Grid3D {
  public:
    const int side_length;

  private:
    std::vector<Cell> raw_grid{};

    int to_index(int x, int y, int z) const {
        return (x * side_length + y) * side_length + z;
    }

    bool in_bounds(int x, int y, int z) const {
        for (int coord : {x, y, z}) {
            if (coord < 0 || coord >= side_length) {
                return false;
            }
        }
        return true;
    }
    bool in_bounds(int index) const {
        return index >= 0 && index < side_length * side_length * side_length;
    }

    void for_each_neighbor(int x, int y, int z,
                           std::function<void(int, int, int)> func) {
        for (int delta : {-1, +1}) {
            for (int *var : {&x, &y, &z}) {
                *var += delta;
                if (in_bounds(x, y, z)) {
                    func(x, y, z);
                }
                *var -= delta;
            }
        }
    }

    void for_each_neighbor(int index, std::function<void(int)> func) {
        for (int delta : {-1, +1}) {
            for (int stride : {1, side_length, side_length * side_length}) {
                index += delta * stride;
                if (in_bounds(index)) {
                    func(index);
                }
                index -= delta * stride;
            }
        }
    }

  public:
    explicit Grid3D(int side_length)
        : side_length(side_length),
          raw_grid(side_length * side_length * side_length) {}

    void set(int x, int y, int z) {
        assert(in_bounds(x, y, z));
        raw_grid[to_index(x, y, z)].lava = true;
    }
    bool is_air(int x, int y, int z, bool exterior) {
        if (!in_bounds(x, y, z)) {
            return true;
        }
        int index = to_index(x, y, z);
        if (!exterior) {
            return !raw_grid[index].lava;
        } else {
            return !raw_grid[index].lava && raw_grid[index].exterior;
        }
    }

    void flood_fill_exterior() {
        // get empty cell on an edge
        int initial_index =
            to_index(side_length - 1, side_length - 1, side_length - 1);
        std::queue<int> pending_indices{{initial_index}};
        while (!pending_indices.empty()) {
            int index = pending_indices.front();
            pending_indices.pop();
            if (raw_grid[index].flood_visited)
                continue;
            raw_grid[index].flood_visited = true;
            if (raw_grid[index].lava)
                continue;
            raw_grid[index].exterior = true;
            for_each_neighbor(
                index, [&](int nindex) { pending_indices.emplace(nindex); });
        }
    }

    int count_exposed_sides(int x, int y, int z, bool exterior) {
        if (is_air(x, y, z, exterior)) {
            return 0;
        }
        int count = 0;
        for_each_neighbor(x, y, z, [=, &count](int nx, int ny, int nz) {
            if (is_air(nx, ny, nz, exterior)) {
                ++count;
            }
        });
        return count;
    }
};

} // namespace aoc::day18

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day18;
    // max value is 21; add 2 extra layers so flood-fill can reach everywhere
    Grid3D grid{21 + 1 + 2};
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream ss{line};
        int x, y, z;
        char c;
        ss >> x >> c >> y >> c >> z;
        grid.set(++x, ++y, ++z);
    }
    int exposed_sides = 0;
    for (int x = 0; x < grid.side_length; ++x) {
        for (int y = 0; y < grid.side_length; ++y) {
            for (int z = 0; z < grid.side_length; ++z) {
                exposed_sides += grid.count_exposed_sides(x, y, z, false);
            }
        }
    }
    std::cout << exposed_sides << std::endl;
    grid.flood_fill_exterior();
    exposed_sides = 0;
    for (int x = 0; x < grid.side_length; ++x) {
        for (int y = 0; y < grid.side_length; ++y) {
            for (int z = 0; z < grid.side_length; ++z) {
                exposed_sides += grid.count_exposed_sides(x, y, z, true);
            }
        }
    }
    std::cout << exposed_sides << std::endl;
    return 0;
}
