/******************************************************************************
 * File:        day22.cpp
 *
 * Author:      yut23
 * Created:     2022-12-27
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for max
#include <cassert>   // for assert
#include <cmath>     // for sqrt
#include <iostream>  // for cout, cerr
#include <map>       // for map
#include <memory>    // for shared_ptr, weak_ptr, enable_shared_from_this,
                     //     unique_ptr, make_unique
#include <compare>   // for strong_ordering
#include <stdexcept> // for logic_error
#include <string>    // for string, getline
#include <utility>   // for move
#include <vector>    // for vector

namespace aoc::day22 {

enum class Facing : char { right = 0, down = 1, left = 2, up = 3 };
constexpr auto FACINGS = {Facing::right, Facing::down, Facing::left,
                          Facing::up};

Facing turn_by(const Facing &current, int amount) {
    return static_cast<Facing>((static_cast<char>(current) + amount + 4) % 4);
}
Facing turn_left(const Facing &current) { return turn_by(current, -1); }
Facing turn_right(const Facing &current) { return turn_by(current, +1); }
Facing get_opposite(const Facing &current) { return turn_by(current, 2); }

Delta to_delta(const Facing &facing) {
    switch (facing) {
    case Facing::up:
        return Delta(0, -1);
    case Facing::down:
        return Delta(0, 1);
    case Facing::right:
        return Delta(1, 0);
    case Facing::left:
        return Delta(-1, 0);
    }
    assert(false);
}

template <typename T>
struct Node : public std::enable_shared_from_this<Node<T>> {
  public: // member types
    using value_type = T;
    using data_pointer = std::unique_ptr<T>;

  private: // member variables
    data_pointer data;

    class Link {
        std::weak_ptr<Node> ptr{};
        Facing facing{};

        using element_type = Node;
        friend Node; // allow Node to access the private constructors

        Link() = default;
        Link(const std::shared_ptr<Node> &r, Facing facing)
            : ptr(r), facing(facing) {}

      public:
        const Facing &new_facing() const {
            assert(ptr.lock());
            return facing;
        }

        element_type &operator*() const { return *ptr.lock(); }
        element_type *get() const { return ptr.lock().get(); }
        element_type *operator->() const { return get(); }

        explicit operator bool() const { return static_cast<bool>(ptr.lock()); }
    };

  public: // public member variables
    // Link north{}, south{}, east{}, west{};
    std::map<Facing, Link> links{};

  private:
    explicit Node(data_pointer data) : data(std::move(data)), links() {
        for (Facing facing : FACINGS) {
            links.emplace(facing, Link{});
        }
    }

  public:
    std::shared_ptr<Node> getptr() { return this->shared_from_this(); }
    [[nodiscard]] static std::shared_ptr<Node> create(data_pointer data) {
        return std::shared_ptr<Node>(new Node(std::move(data)));
    }

    void link_to(std::shared_ptr<Node> other, Facing link_side,
                 Facing new_facing);

    T &operator*() const { return *data; }
    T *operator->() const { return data.get(); }

    const Link &north() const { return links.at(Facing::up); }
    const Link &south() const { return links.at(Facing::down); }
    const Link &east() const { return links.at(Facing::right); }
    const Link &west() const { return links.at(Facing::left); }
};

template <typename T>
void Node<T>::link_to(std::shared_ptr<Node> other, Facing link_side,
                      Facing new_facing) {
    links.at(link_side) = {other, new_facing};
}

template <typename T>
class LinkedGrid {
  public: // member types
    using value_type = T;
    using data_pointer = std::unique_ptr<T>;
    using node_type = Node<T>;
    using node_pointer = std::shared_ptr<node_type>;

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

    typename decltype(nodes)::size_type size() const { return nodes.size(); }
};

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
    auto node = node_type::create(std::move(data));
    nodes.emplace(pos, node);
    // link adjacent nodes
    for (Facing facing : FACINGS) {
        auto other = nodes.find(pos + to_delta(facing));
        if (other != nodes.end()) {
            node->link_to(other->second, facing, facing);
            Facing opposite = get_opposite(facing);
            other->second->link_to(node, opposite, opposite);
        }
    }
    return node;
}

struct NodeData {
    const bool wall;
    const Pos pos;

    NodeData(bool wall, const Pos &pos) : wall(wall), pos(pos) {}
};

struct PathFollower {
    const LinkedGrid<NodeData>::node_type *curr_node;
    Facing facing;

    explicit PathFollower(const LinkedGrid<NodeData>::node_type *starting_node)
        : curr_node(starting_node), facing(Facing::right) {}

    void follow_path(std::istream &is);
    void move_forward();

    int get_password() const {
        return 1000 * ((*curr_node)->pos.y + 1) +
               4 * ((*curr_node)->pos.x + 1) + static_cast<char>(facing);
    }
};

void PathFollower::move_forward() {
    auto link = curr_node->links.at(facing);
    assert(link);
    if (!(*link)->wall) {
        [[maybe_unused]] Facing prev_facing = facing;
        facing = link.new_facing();
        if constexpr (aoc::DEBUG) {
            if (prev_facing != facing) {
                std::cerr << "crossing disconnected edge from "
                          << (*curr_node)->pos << " to ";
            }
        }
        curr_node = link.get();
        if constexpr (aoc::DEBUG) {
            if (prev_facing != facing) {
                std::cerr << (*curr_node)->pos << "\n";
            }
        }
    }
}

void PathFollower::follow_path(std::istream &is) {
    // get next character
    char c;
    while (is.get(c)) {
        if (c == '\n') {
            break;
        }
        if (c == 'L') {
            if constexpr (aoc::DEBUG) {
                std::cerr << "turning " << c << " at " << (*curr_node)->pos
                          << "\n";
            }
            facing = turn_left(facing);
        } else if (c == 'R') {
            if constexpr (aoc::DEBUG) {
                std::cerr << "turning " << c << " at " << (*curr_node)->pos
                          << "\n";
            }
            facing = turn_right(facing);
        } else {
            // put digit back
            is.unget();
            int count;
            is >> count;
            if constexpr (aoc::DEBUG) {
                std::cerr << "moving forward " << count << " spaces from "
                          << (*curr_node)->pos << "\n";
            }
            for (int i = 0; i < count; ++i) {
                move_forward();
            }
        }
    }
    if constexpr (aoc::DEBUG) {
        std::cerr << "finished at " << (*curr_node)->pos << " with facing "
                  << static_cast<int>(facing) << "\n";
    }
}

const LinkedGrid<NodeData>::node_type *read_part_1(LinkedGrid<NodeData> &grid,
                                                   std::istream &infile) {
    using node_pointer = LinkedGrid<NodeData>::node_pointer;
    // read file line-by-line
    std::string line;
    std::map<int, node_pointer> first_nodes_in_column{};
    node_pointer first_node_in_line{};
    node_pointer starting_node{};
    int y = 0;
    int max_x = 0;
    while (std::getline(infile, line)) {
        max_x = std::max(max_x, static_cast<int>(line.size()));
        for (int x = 0; x <= max_x; ++x) {
            if (x >= static_cast<long>(line.size()) || line[x] == ' ') {
                if (first_node_in_line) {
                    // link first node and previous node
                    auto right_node = grid.get_node(Pos(x - 1, y));
                    assert(right_node);
                    if constexpr (aoc::DEBUG) {
                        std::cerr << "linking node at " << (*right_node)->pos
                                  << " on right edge to node at "
                                  << (*first_node_in_line)->pos << "\n";
                    }
                    right_node->link_to(first_node_in_line, Facing::right,
                                        Facing::right);
                    first_node_in_line->link_to(right_node, Facing::left,
                                                Facing::left);
                    first_node_in_line.reset();
                }
                if (first_nodes_in_column.contains(x)) {
                    // link first node and previous node
                    auto bottom_node = grid.get_node(Pos(x, y - 1));
                    assert(bottom_node);
                    if constexpr (aoc::DEBUG) {
                        std::cerr << "linking node at " << (*bottom_node)->pos
                                  << " on bottom edge to node at "
                                  << (*first_nodes_in_column.at(x))->pos
                                  << "\n";
                    }
                    bottom_node->link_to(first_nodes_in_column.at(x),
                                         Facing::down, Facing::down);
                    first_nodes_in_column.at(x)->link_to(
                        bottom_node, Facing::up, Facing::up);
                    first_nodes_in_column.erase(x);
                }
            } else {
                Pos pos{x, y};
                if constexpr (aoc::DEBUG && false) {
                    std::cerr << "adding node at " << pos << "\n";
                }
                auto node = grid.add_node(
                    pos, std::make_unique<NodeData>(line[x] == '#', pos));
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
    return starting_node.get();
}

struct FaceLink {
    const int face_index;
    const Facing facing;

    FaceLink(int face_index, Facing facing)
        : face_index(face_index), facing(facing) {}

    std::strong_ordering operator<=>(const FaceLink &other) const = default;
};
inline FaceLink operator+(FaceLink lhs, int rhs) {
    return {lhs.face_index, turn_by(lhs.facing, rhs)};
}
inline FaceLink operator-(FaceLink lhs, int rhs) {
    return {lhs.face_index, turn_by(lhs.facing, -rhs)};
}

std::ostream &operator<<(std::ostream &os,
                         const std::map<FaceLink, FaceLink> &face_connections) {
    os << "    N    E    S    W\n";
    for (int index = 0; index < 6; ++index) {
        os << index + 1;
        for (Facing facing :
             {Facing::up, Facing::right, Facing::down, Facing::left}) {
            os << "  ";
            FaceLink src_link{index, facing};
            auto it = face_connections.find(src_link);
            if (it == face_connections.end()) {
                os << "   ";
            } else {
                os << it->second.face_index + 1 << "/";
                switch (it->second.facing) {
                case Facing::up:
                    os << "N";
                    break;
                case Facing::down:
                    os << "S";
                    break;
                case Facing::right:
                    os << "E";
                    break;
                case Facing::left:
                    os << "W";
                    break;
                }
            }
        }
        os << "\n";
    }
    return os;
}

void link_faces(std::map<FaceLink, FaceLink> &face_connections) {
    // a cube has 12 edges, each of which we can cross in two directions
    while (face_connections.size() < 12 * 2) {
        // loop over the 6 faces
        for (int index = 0; index < 6; ++index) {
            for (Facing facing : FACINGS) {
                FaceLink base{index, facing};
                auto base_it = face_connections.find(base);
                if (base_it == face_connections.end()) {
                    continue;
                }
                for (int shift : {-1, 1}) {
                    if (face_connections.contains(base + shift)) {
                        // skip links we've already got
                        continue;
                    }
                    FaceLink current = face_connections.at(base);
                    auto it = face_connections.find(current + shift);
                    if (it == face_connections.end()) {
                        continue;
                    }
                    face_connections.emplace(base + shift, it->second - shift);
                }
            }
        }
    }
}

const LinkedGrid<NodeData>::node_type *
read_part_2(LinkedGrid<NodeData> &grid, std::istream &infile, int face_width) {
    using node_pointer = LinkedGrid<NodeData>::node_pointer;

    // read file line-by-line
    std::string line;
    std::map<Pos, int> face_indices{};
    std::vector<Pos> face_positions{};
    // [source face index][direction to move] = {dest face index, new facing}
    std::map<FaceLink, FaceLink> face_connections{};
    node_pointer starting_node{};
    int y = 0;
    while (std::getline(infile, line)) {
        for (int x = 0; x < static_cast<int>(line.size()); ++x) {
            Pos pos{x, y};
            if (line[x] == ' ') {
                continue;
            }
            auto node = grid.add_node(
                pos, std::make_unique<NodeData>(line[x] == '#', pos));
            Pos face_pos = pos / face_width;
            if (!face_indices.contains(face_pos)) {
                int source_index = face_positions.size();
                face_positions.emplace_back(face_pos);
                face_indices.emplace(face_pos, source_index);
                // link adjacent faces
                for (const Facing &dir_to_move : {Facing::left, Facing::up}) {
                    Pos other_pos = face_pos + to_delta(dir_to_move);
                    auto it = face_indices.find(other_pos);
                    if (it != face_indices.end()) {
                        int dest_index = it->second;
                        Facing opposite = get_opposite(dir_to_move);
                        // add links in both directions
                        face_connections.emplace(
                            FaceLink(source_index, dir_to_move),
                            FaceLink(dest_index, dir_to_move));
                        face_connections.emplace(
                            FaceLink(dest_index, opposite),
                            FaceLink(source_index, opposite));
                    }
                }
            }
            if (!starting_node) {
                starting_node = node;
            }
        }
        if (line.empty()) {
            break;
        }
        ++y;
    }
    assert(face_positions.size() == 6);
    assert(face_indices.size() == 6);
    // fill in missing links in face connections
    if constexpr (aoc::DEBUG) {
        std::cerr << "initial:\n" << face_connections << "\n";
    }
    assert(face_connections.size() == 10);
    link_faces(face_connections);
    if constexpr (aoc::DEBUG) {
        std::cerr << "final:\n" << face_connections << "\n";
    }

    // TODO: link nodes according to face_connections
    for (auto &[pos, node] : grid) {
        for (Facing facing : FACINGS) {
            if (!node->links.at(facing)) {
                // get position along edge
                Pos source_face_pos = pos / face_width;
                // relative position along edge, starting from the left as seen
                // when oriented towards `facing`
                int rel_pos;
                switch (facing) {
                case Facing::up:
                    rel_pos = pos.x - source_face_pos.x * face_width;
                    break;
                case Facing::down:
                    rel_pos = (source_face_pos.x + 1) * face_width - pos.x - 1;
                    break;
                case Facing::right:
                    rel_pos = pos.y - source_face_pos.y * face_width;
                    break;
                case Facing::left:
                    rel_pos = (source_face_pos.y + 1) * face_width - pos.y - 1;
                }
                assert(rel_pos >= 0 && rel_pos < face_width);

                int source_index = face_indices.at(source_face_pos);
                const FaceLink &dest =
                    face_connections.at({source_index, facing});
                const Pos &dest_face_pos = face_positions[dest.face_index];

                Delta dest_shift{0, 0};
                switch (dest.facing) {
                case Facing::up:
                    // bottom edge
                    dest_shift = {rel_pos, face_width - 1};
                    break;
                case Facing::down:
                    // top edge, with x reversed
                    dest_shift = {face_width - rel_pos - 1, 0};
                    break;
                case Facing::right:
                    // left edge
                    dest_shift = {0, rel_pos};
                    break;
                case Facing::left:
                    // right edge, with y reversed
                    dest_shift = {face_width - 1, face_width - rel_pos - 1};
                    break;
                }
                Pos dest_pos = dest_face_pos * face_width + dest_shift;
                LinkedGrid<NodeData>::node_pointer dest_node =
                    grid.get_node(dest_pos);
                node->link_to(dest_node, facing, dest.facing);
            }
        }
    }

    return starting_node.get();
}

} // namespace aoc::day22

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day22;
    LinkedGrid<NodeData> grid{};
    const LinkedGrid<NodeData>::node_type *starting_node =
        read_part_1(grid, infile);
    // check that all nodes are fully connected
    if constexpr (aoc::DEBUG) {
        for (const auto &[pos, node] : grid) {
            assert(pos == (*node)->pos);
            assert(node->links.at(Facing::up));
            assert(node->links.at(Facing::down));
            assert(node->links.at(Facing::right));
            assert(node->links.at(Facing::left));
        }
    }
    PathFollower pf{starting_node};
    pf.follow_path(infile);
    std::cout << pf.get_password() << "\n";

    int face_width = std::sqrt(grid.size() / 6);
    infile.clear();
    infile.seekg(0);
    grid = LinkedGrid<NodeData>();
    if constexpr (aoc::DEBUG) {
        std::cerr << "\n";
    }
    starting_node = read_part_2(grid, infile, face_width);
    // check that all nodes are fully connected
    if constexpr (aoc::DEBUG) {
        for (const auto &[pos, node] : grid) {
            assert(pos == (*node)->pos);
            assert(node->links.at(Facing::up));
            assert(node->links.at(Facing::down));
            assert(node->links.at(Facing::right));
            assert(node->links.at(Facing::left));
        }
    }
    pf = PathFollower{starting_node};
    pf.follow_path(infile);
    std::cout << pf.get_password() << "\n";
    return 0;
}
