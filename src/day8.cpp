/******************************************************************************
 * File:        day8.cpp
 *
 * Author:      yut23
 * Created:     2022-12-08
 *****************************************************************************/

#include "lib.h"
#include <algorithm>  // for count_if
#include <cassert>    // for assert
#include <concepts>   // for same_as
#include <cstdint>    // for uint8_t
#include <functional> // for plus
#include <iostream>   // for cout
#include <iterator>   // for input_iterator, sentinel_for, iter_value_t
#include <memory>     // for shared_ptr, make_shared, pointer_traits
#include <numeric>    // for transform_reduce
#include <string>     // for string, getline
#include <vector>     // for vector

namespace aoc::day8 {

struct Tree {
    // pack the height and a visible flag into one byte (if the compiler
    // cooperates) 4 bits can hold from 0 to 15; we only need 0 to 9
    std::uint8_t height : 4;
    bool visible : 1 {false};

    explicit Tree(std::uint8_t height_) : height(height_) {}
};
static_assert(sizeof(Tree) == 1, "tree struct not packed");

template <std::input_iterator I, std::sentinel_for<I> S>
requires std::same_as<
    typename std::pointer_traits<std::iter_value_t<I>>::element_type, Tree>
void mark_visible(I first, S last) {
    std::uint8_t tallest = (*first)->height;
    for (auto it = first; it != last; ++it) {
        // the first tree is always visible
        if (it == first || (*it)->height > tallest) {
            (*it)->visible = true;
            tallest = (*it)->height;
        }
    }
}

class Forest {
  private:
    std::vector<std::vector<std::shared_ptr<Tree>>> trees_row_major{};
    std::vector<std::vector<std::shared_ptr<Tree>>> trees_col_major{};

  public:
    void add_row(const std::string &line);
    // mark the trees that are visible from any direction
    void mark();
    int count_visible() const;
};

void Forest::add_row(const std::string &line) {
    if (trees_row_major.empty()) {
        assert(trees_col_major.empty());
        // initialize column major view
        trees_col_major.resize(line.length());
    }
    assert(trees_col_major.size() == line.length());
    // add a new row
    trees_row_major.emplace_back();
    auto col_it = trees_col_major.begin();
    for (auto char_it = line.cbegin(); char_it != line.cend();
         ++char_it, ++col_it) {
        auto tree = std::make_shared<Tree>(*char_it - '0');
        // add the tree to the last row
        trees_row_major.back().push_back(tree);
        // add the tree to the end of the corresponding column
        col_it->push_back(tree);
    }
}

void Forest::mark() {
    for (const auto &row : trees_row_major) {
        mark_visible(row.begin(), row.end());
        mark_visible(row.rbegin(), row.rend());
    }
    for (const auto &col : trees_col_major) {
        mark_visible(col.begin(), col.end());
        mark_visible(col.rbegin(), col.rend());
    }
}

int Forest::count_visible() const {
    return std::transform_reduce(
        trees_row_major.cbegin(), trees_row_major.cend(), 0, std::plus{},
        [](const auto &row) {
            return std::ranges::count_if(
                row, [](const auto &tree) { return tree->visible; });
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
    forest.mark();
    std::cout << forest.count_visible() << std::endl;
    return 0;
}
