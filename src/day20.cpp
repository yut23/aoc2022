/******************************************************************************
 * File:        day20.cpp
 *
 * Author:      yut23
 * Created:     2022-12-20
 *****************************************************************************/

#include "lib.h"
#include <cstdlib>  // for abs
#include <iostream> // for cout
#include <memory>   // for unique_ptr, make_unique
#include <string>   // for string, getline
#include <utility>  // for move
#include <vector>   // for vector

namespace aoc::day20 {
template <typename T>
struct CircularLinkedNode {
    T data;
    CircularLinkedNode<T> *next = this;
    CircularLinkedNode<T> *prev = this;

    explicit CircularLinkedNode(const T &data) : data(data) {}

    void insert_before(CircularLinkedNode<T> *);
    void insert_after(CircularLinkedNode<T> *);
    void unlink();
};

template <typename T>
void CircularLinkedNode<T>::insert_after(CircularLinkedNode<T> *new_node) {
    auto *other = this->next;
    this->next = new_node;
    new_node->next = other;
    other->prev = new_node;
    new_node->prev = this;
}

template <typename T>
void CircularLinkedNode<T>::insert_before(CircularLinkedNode<T> *new_node) {
    auto *other = this->prev;
    other->next = new_node;
    new_node->next = this;
    this->prev = new_node;
    new_node->prev = other;
}

template <typename T>
void CircularLinkedNode<T>::unlink() {
    this->next->prev = this->prev;
    this->prev->next = this->next;
}

template <typename T>
void print_list(std::ostream &os, CircularLinkedNode<T> *head) {
    auto *ptr = head;
    while (true) {
        os << ptr->data;
        ptr = ptr->next;
        if (ptr == head) {
            break;
        } else {
            os << ", ";
        }
    }
    os << "\n";
}

} // namespace aoc::day20

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day20;
    // need to store pointers here since the vector may change capacity
    std::vector<std::unique_ptr<CircularLinkedNode<int>>> nodes;
    // read file
    int value;
    CircularLinkedNode<int> *zero;
    while (infile >> value) {
        auto node_ptr = std::make_unique<CircularLinkedNode<int>>(value);
        if (nodes.size() > 0) {
            nodes.back()->insert_after(node_ptr.get());
        }
        if (value == 0) {
            zero = node_ptr.get();
        }
        nodes.emplace_back(std::move(node_ptr));
    }

    CircularLinkedNode<int> *head = nodes.front().get();
    if constexpr (aoc::DEBUG) {
        std::cerr << "initial arangement:\n";
    }
    for (const auto &node : nodes) {
        if constexpr (aoc::DEBUG) {
            print_list(std::cerr, head);
            std::cerr << "\nmoving " << node->data << ":\n";
        }
        if (node->data == 0) {
            continue;
        }
        auto *ptr = node.get();
        if (ptr == head) {
            head = ptr->next;
        }
        node->unlink();
        for (int i = 0; i < std::abs(node->data); ++i) {
            if (node->data > 0) {
                ptr = ptr->next;
            } else {
                ptr = ptr->prev;
            }
        }
        if (ptr->next == head) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "(updated head)\n";
            }
            head = node.get();
        }
        if (node->data > 0) {
            ptr->insert_after(node.get());
        } else {
            ptr->insert_before(node.get());
        }
    }
    if constexpr (aoc::DEBUG) {
        print_list(std::cerr, head);
    }

    int sum = 0;
    for (int i = 1; i <= 3; ++i) {
        auto *ptr = zero;
        for (int index = (i * 1000) % nodes.size(); index > 0; --index) {
            ptr = ptr->next;
        }
        sum += ptr->data;
    }
    std::cout << sum << "\n";
    return 0;
}
