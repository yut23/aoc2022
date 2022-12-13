/******************************************************************************
 * File:        day12.cpp
 *
 * Author:      yut23
 * Created:     2022-12-12
 *****************************************************************************/

#include "lib.h"
#include <iostream>  // for cout
#include <limits>    // for numeric_limits
#include <map>       // for map
#include <memory>    // for unique_ptr
#include <queue>     // for queue
#include <stdexcept> // for logic_error
#include <utility>   // for move

namespace aoc::day12 {

template <typename T>
class LinkedGrid {
  public: // member types
    using value_type = T;
    using data_pointer = std::unique_ptr<T>;

    struct Node {
      private:
        data_pointer data;

      public:
        std::weak_ptr<Node> north{}, south{}, east{}, west{};

      public:
        explicit Node(data_pointer data) : data(std::move(data)) {}

        T &operator*() { return *data; }
        const T &operator*() const { return *data; }
        T *operator->() { return data.get(); }
        const T *operator->() const { return data.get(); }
    };

    using node_pointer = std::shared_ptr<Node>;

  private:
    std::map<Pos, node_pointer> nodes{};

  public:
    node_pointer add_node(const Pos &, data_pointer);
};

template <typename T>
typename LinkedGrid<T>::node_pointer
LinkedGrid<T>::add_node(const Pos &pos, LinkedGrid<T>::data_pointer data) {
    if (nodes.contains(pos)) {
        throw std::logic_error("Grid node already exists at this position");
    }
    auto node = make_shared<Node>(std::move(data));
    nodes.emplace(pos, node);
    // link adjacent nodes
    for (Direction dir :
         {Direction::up, Direction::down, Direction::right, Direction::left}) {
        auto other = nodes.find(pos + Delta(dir));
        if (other != nodes.end()) {
            switch (dir) {
            case Direction::up:
                node->north = other->second;
                other->second->south = node;
                break;
            case Direction::down:
                node->south = other->second;
                other->second->north = node;
                break;
            case Direction::right:
                node->east = other->second;
                other->second->west = node;
                break;
            case Direction::left:
                node->west = other->second;
                other->second->east = node;
                break;
            }
        }
    }
    return node;
}

struct NodeData {
    int height;
    int distance{std::numeric_limits<decltype(distance)>::max()};
    bool visited{false};

    explicit NodeData(int height) : height(height) {}
};

int bfs(LinkedGrid<NodeData>::node_pointer &end) {
    std::queue<LinkedGrid<NodeData>::node_pointer> queue{{end}};
    (*end)->distance = 0;
    int curr_distance = -1;
    int closest_a = std::numeric_limits<int>::max();
    while (!queue.empty()) {
        LinkedGrid<NodeData>::Node &node = *queue.front();
        queue.pop();
        if (node->visited) {
            continue;
        }
        node->visited = true;
        if (node->height == 0 && node->distance < closest_a) {
            closest_a = node->distance;
        }
        if (node->distance > curr_distance) {
            curr_distance = node->distance;
            if constexpr (aoc::DEBUG) {
                std::cerr << "now processing distance " << curr_distance
                          << " nodes..." << std::endl;
            }
        }

        for (auto weak_neighbor :
             {node.north, node.east, node.south, node.west}) {
            if (auto neighbor_ptr = weak_neighbor.lock()) {
                LinkedGrid<NodeData>::Node &neighbor = *neighbor_ptr;
                // neighbor exists
                if (node->height > neighbor->height + 1) {
                    // too steep
                    continue;
                }
                if (neighbor->distance <= node->distance) {
                    // already visited
                    continue;
                }
                // update the neighbor's distance from the end and add it to
                // the queue
                neighbor->distance = node->distance + 1;
                queue.push(neighbor_ptr);
            }
        }
    }
    return closest_a;
}

} // namespace aoc::day12

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day12;
    LinkedGrid<NodeData> grid{};
    LinkedGrid<NodeData>::node_pointer start{};
    LinkedGrid<NodeData>::node_pointer end{};

    aoc::Pos start_of_line{0, 0};
    aoc::Pos curr_pos{start_of_line};
    // read file character-by-character
    char c;
    while (infile.get(c)) {
        if (c == '\n') {
            start_of_line += aoc::Delta(aoc::Direction::down);
            curr_pos = start_of_line;
            continue;
        }
        int height = c - 'a';
        if (c == 'S') {
            height = 0;
        } else if (c == 'E') {
            height = 'z' - 'a';
        }
        auto node = grid.add_node(curr_pos, std::make_unique<NodeData>(height));
        if (c == 'S') {
            start = node;
        } else if (c == 'E') {
            end = node;
        }
        curr_pos += aoc::Delta(aoc::Direction::right);
    }
    // perform a BFS over the grid starting from `end`
    // at the same time, find the lowest distance among nodes with height == 0
    int closest_a = bfs(end);

    std::cout << (*start)->distance << std::endl;
    std::cout << closest_a << std::endl;

    return 0;
}
