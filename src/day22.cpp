/******************************************************************************
 * File:        day22.cpp
 *
 * Author:      yut23
 * Created:     2022-12-27
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for max
#include <cassert>   // for assert
#include <iostream>  // for cout, cerr
#include <map>       // for map
#include <memory>    // for shared_ptr, weak_ptr, enable_shared_from_this,
                     //     unique_ptr, make_unique
#include <sstream>   // for istringstream
#include <stdexcept> // for logic_error
#include <string>    // for string, getline
#include <utility>   // for move

namespace aoc::day22 {

enum class Facing : char { right = 0, down = 1, left = 2, up = 3 };

Facing turn_left(const Facing &current) {
    return static_cast<Facing>((static_cast<char>(current) + 4 - 1) % 4);
}
Facing turn_right(const Facing &current) {
    return static_cast<Facing>((static_cast<char>(current) + 1) % 4);
}

template <typename T>
class LinkedGrid {
  public: // member types
    using value_type = T;
    using data_pointer = std::unique_ptr<T>;

    struct Node : public std::enable_shared_from_this<Node> {
      private:
        data_pointer data;

      public:
        std::weak_ptr<Node> north{}, south{}, east{}, west{};

      private:
        explicit Node(data_pointer data) : data(std::move(data)) {}

      public:
        std::shared_ptr<Node> getptr() { return this->shared_from_this(); }
        [[nodiscard]] static std::shared_ptr<Node> create(data_pointer data) {
            return std::shared_ptr<Node>(new Node(std::move(data)));
        }

        void link_to(std::shared_ptr<Node> other, Direction dir);

        T &operator*() const { return *data; }
        T *operator->() const { return data.get(); }
    };

    using node_t = Node;
    using node_pointer = std::shared_ptr<Node>;

  private:
    std::map<Pos, node_pointer> nodes{};

  public:
    node_pointer get_node(const Pos &);
    node_pointer add_node(const Pos &, data_pointer);

    typename decltype(nodes)::const_iterator begin() const {
        return nodes.cbegin();
    }
    typename decltype(nodes)::const_iterator end() const {
        return nodes.cend();
    }
    typename decltype(nodes)::const_iterator cbegin() const {
        return nodes.cbegin();
    }
    typename decltype(nodes)::const_iterator cend() const {
        return nodes.cend();
    }
};

template <typename T>
void LinkedGrid<T>::Node::link_to(node_pointer other, Direction dir) {
    switch (dir) {
    case Direction::up:
        assert(!north.lock());
        north = other;
        assert(!other->south.lock());
        other->south = getptr();
        break;
    case Direction::down:
        assert(!south.lock());
        south = other;
        assert(!other->north.lock());
        other->north = getptr();
        break;
    case Direction::right:
        assert(!east.lock());
        east = other;
        assert(!other->west.lock());
        other->west = getptr();
        break;
    case Direction::left:
        assert(!west.lock());
        west = other;
        assert(!other->east.lock());
        other->east = getptr();
        break;
    }
}

template <typename T>
auto LinkedGrid<T>::get_node(const Pos &pos) -> node_pointer {
    auto it = nodes.find(pos);
    if (it != nodes.end()) {
        return it->second;
    }
    return {};
}

template <typename T>
auto LinkedGrid<T>::add_node(const Pos &pos, data_pointer data)
    -> node_pointer {
    if (nodes.contains(pos)) {
        throw std::logic_error("Grid node already exists at this position");
    }
    auto node = Node::create(std::move(data));
    nodes.emplace(pos, node);
    // link adjacent nodes
    for (Direction dir :
         {Direction::up, Direction::down, Direction::right, Direction::left}) {
        auto other = nodes.find(pos + Delta(dir, true));
        if (other != nodes.end()) {
            node->link_to(other->second, dir);
        }
    }
    return node;
}

struct NodeData {
    const bool wall;
    const Pos pos;

    NodeData(bool wall, const Pos &pos) : wall(wall), pos(pos) {}
};

const LinkedGrid<NodeData>::node_t *
move_forward(const LinkedGrid<NodeData>::node_t *node, Facing facing,
             int count) {
    for (int i = 0; i < count; ++i) {
        LinkedGrid<NodeData>::node_pointer next;
        switch (facing) {
        case Facing::up:
            next = node->north.lock();
            break;
        case Facing::down:
            next = node->south.lock();
            break;
        case Facing::right:
            next = node->east.lock();
            break;
        case Facing::left:
            next = node->west.lock();
            break;
        }
        assert(next);
        if ((*next)->wall) {
            return node;
        }
        node = next.get();
    }
    return node;
}

} // namespace aoc::day22

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day22;
    LinkedGrid<NodeData> grid{};
    using node_pointer = LinkedGrid<NodeData>::node_pointer;
    // read file line-by-line
    std::string line;
    std::map<int, node_pointer> first_nodes_in_column{};
    node_pointer first_node_in_line{};
    node_pointer starting_node{};
    int y = 1;
    int max_x = 1;
    while (std::getline(infile, line)) {
        max_x = std::max(max_x, static_cast<int>(line.size()));
        for (int x = 1; x <= max_x + 1; ++x) {
            if (x - 1 >= static_cast<long>(line.size()) || line[x - 1] == ' ') {
                if (first_node_in_line) {
                    // link first node and previous node
                    auto right_node = grid.get_node(aoc::Pos(x - 1, y));
                    assert(right_node);
                    if constexpr (aoc::DEBUG) {
                        std::cerr << "linking node at " << (*right_node)->pos
                                  << " on right edge to node at "
                                  << (*first_node_in_line)->pos << "\n";
                    }
                    right_node->link_to(first_node_in_line,
                                        aoc::Direction::right);
                    first_node_in_line.reset();
                }
                if (first_nodes_in_column.contains(x)) {
                    // link first node and previous node
                    auto bottom_node = grid.get_node(aoc::Pos(x, y - 1));
                    assert(bottom_node);
                    if constexpr (aoc::DEBUG) {
                        std::cerr << "linking node at " << (*bottom_node)->pos
                                  << " on bottom edge to node at "
                                  << (*first_nodes_in_column.at(x))->pos
                                  << "\n";
                    }
                    bottom_node->link_to(first_nodes_in_column.at(x),
                                         aoc::Direction::down);
                    first_nodes_in_column.erase(x);
                }
            } else {
                aoc::Pos pos{x, y};
                if constexpr (aoc::DEBUG) {
                    std::cerr << "adding node at " << pos << "\n";
                }
                auto node = grid.add_node(
                    pos, std::make_unique<NodeData>(line[x - 1] == '#', pos));
                if (!starting_node) {
                    starting_node = node;
                }
                if (!first_node_in_line) {
                    first_node_in_line = node;
                }
                if (!first_nodes_in_column.contains(x)) {
                    first_nodes_in_column.emplace(x, node);
                }
            }
        }
        if (line.empty()) {
            break;
        }
        ++y;
    }
    // check that all nodes are fully connected
    if constexpr (aoc::DEBUG) {
        for (const auto &[pos, node] : grid) {
            assert(pos == (*node)->pos);
            assert(node->north.lock());
            assert(node->south.lock());
            assert(node->east.lock());
            assert(node->west.lock());
        }
    }
    // got empty line between map and path
    std::getline(infile, line);
    // follow path
    std::istringstream ss{line};
    const LinkedGrid<NodeData>::node_t *curr_node = starting_node.get();
    Facing facing = Facing::right;
    int count;
    char dir;
    ss >> count;
    curr_node = move_forward(curr_node, facing, count);
    while (ss >> dir >> count) {
        if constexpr (aoc::DEBUG) {
            std::cerr << "turning " << dir << " at " << (*curr_node)->pos
                      << "\n";
        }
        if (dir == 'L') {
            facing = turn_left(facing);
        } else if (dir == 'R') {
            facing = turn_right(facing);
        } else {
            assert(false);
        }
        curr_node = move_forward(curr_node, facing, count);
    }
    if constexpr (aoc::DEBUG) {
        std::cerr << "finished at " << (*curr_node)->pos << " with facing "
                  << static_cast<int>(facing) << "\n";
    }
    int password = 1000 * (*curr_node)->pos.y + 4 * (*curr_node)->pos.x +
                   static_cast<char>(facing);
    std::cout << password << "\n";
    return 0;
}
