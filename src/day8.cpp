/******************************************************************************
 * File:        day8.cpp
 *
 * Author:      yut23
 * Created:     2022-12-08
 *****************************************************************************/

#include "lib.h"
#include <algorithm>  // for count_if
#include <cassert>    // for assert
#include <cstdint>    // for uint8_t
#include <functional> // for plus
#include <iostream>   // for cout
#include <numeric>    // for transform_reduce
#include <string>     // for string, getline
#include <vector>     // for vector

namespace aoc::day8 {

struct Tree {
    // pack the height and a visible flag into one byte
    // 4 bits can hold 0 to 15; we only need 0 to 9
    std::uint8_t height : 4;
    bool visible : 1 {false};

    explicit Tree(std::uint8_t height_) : height(height_) {}
};

class Forest {
  private:
    std::vector<std::vector<Tree>> trees{};
    int side_length = 0;

  public:
    int size() const { return side_length; }
    void add_row(const std::string &line);
    // Update the visibility of the tree at (r, c) given a tree of height
    // `tallest` in front of it. May also update `tallest`.
    void update_visibility(int r, int c, int &tallest);
    // mark the trees that are visible from any direction
    void mark_visible();
    int count_visible() const;
};

void Forest::add_row(const std::string &line) {
    // add a new row
    trees.emplace_back();
    ++side_length;
    for (char c : line) {
        // add the tree to the last row
        trees.back().emplace_back(c - '0');
    }
}

void Forest::update_visibility(int row, int col, int &tallest) {
    auto &tree = trees[row][col];
    if (tree.height > tallest) {
        tree.visible = true;
        tallest = tree.height;
    }
}

void Forest::mark_visible() {
    assert(size() > 0);
    assert(size() == trees.size() && size() == trees[0].size());
    for (int i = 0; i < size(); ++i) {
        int tallest_row = -1, tallest_col = -1;
        for (int j = 0; j < size(); ++j) {
            update_visibility(i, j, tallest_row);
            update_visibility(j, i, tallest_col);
        }
    }
    // same thing, but in reverse
    for (int i = size() - 1; i >= 0; --i) {
        int tallest_row = -1, tallest_col = -1;
        for (int j = size() - 1; j >= 0; --j) {
            update_visibility(i, j, tallest_row);
            update_visibility(j, i, tallest_col);
        }
    }
}

int Forest::count_visible() const {
    return std::transform_reduce(
        trees.cbegin(), trees.cend(), 0, std::plus{}, [](const auto &row) {
            return std::ranges::count_if(
                row, [](const auto &tree) { return tree.visible; });
        });
}

} // namespace aoc::day8

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day8;
    Forest forest{};

    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        // construct tree
        forest.add_row(line);
    }
    forest.mark_visible();
    std::cout << forest.count_visible() << std::endl;
    return 0;
}
