/******************************************************************************
 * File:        day18.cpp
 *
 * Author:      yut23
 * Created:     2022-12-18
 *****************************************************************************/

#include "lib.h"
#include <iostream> // for cout
#include <sstream>  // for istringstream
#include <string>   // for string, getline
#include <vector>   // for vector

namespace aoc::day18 {

class Grid3D {
  public:
    const int side_length;

  private:
    std::vector<bool> raw_grid{};

    constexpr int to_index(int x, int y, int z) const {
        return (x * side_length + y) * side_length + z;
    }

    constexpr bool in_bounds(int x, int y, int z) const {
        for (const int &coord : {x, y, z}) {
            if (coord < 0 || coord >= side_length) {
                return false;
            }
        }
        return true;
    }

  public:
    explicit Grid3D(int side_length)
        : side_length(side_length),
          raw_grid(side_length * side_length * side_length) {}

    void set(int x, int y, int z) {
        assert(in_bounds(x, y, z));
        raw_grid[to_index(x, y, z)] = true;
    }
    bool get(int x, int y, int z) {
        return in_bounds(x, y, z) && raw_grid[to_index(x, y, z)];
    }

    int count_exposed_sides(int x, int y, int z) {
        if (!get(x, y, z)) {
            return 0;
        }
        int count = 0;
        for (int delta : {-1, +1}) {
            if (!get(x + delta, y, z))
                ++count;
            if (!get(x, y + delta, z))
                ++count;
            if (!get(x, y, z + delta))
                ++count;
        }
        return count;
    }
};

} // namespace aoc::day18

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day18;
    Grid3D grid{22};
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream ss{line};
        int x, y, z;
        char c;
        ss >> x >> c >> y >> c >> z;
        grid.set(x, y, z);
    }
    int exposed_sides = 0;
    for (int x = 0; x < grid.side_length; ++x) {
        for (int y = 0; y < grid.side_length; ++y) {
            for (int z = 0; z < grid.side_length; ++z) {
                exposed_sides += grid.count_exposed_sides(x, y, z);
            }
        }
    }
    std::cout << exposed_sides << std::endl;
    return 0;
}
