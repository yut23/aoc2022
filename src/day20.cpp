/******************************************************************************
 * File:        day20.cpp
 *
 * Author:      yut23
 * Created:     2022-12-20
 *****************************************************************************/

#include "lib.h"
#include <cstdlib>  // for abs
#include <iostream> // for cout, cerr
#include <memory>   // for unique_ptr, make_unique
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
CircularLinkedNode<T> *advance(CircularLinkedNode<T> *ptr, long count = 1) {
    if (count > 0) {
        for (long i = 0; i < std::abs(count); ++i) {
            ptr = ptr->next;
        }
    } else {
        for (long i = 0; i < std::abs(count); ++i) {
            ptr = ptr->prev;
        }
    }
    return ptr;
}

template <typename T>
class CircularLinkedList {
  public:
    using Node = CircularLinkedNode<T>;

  private:
    using UniquePtr = std::unique_ptr<Node>;
    std::vector<UniquePtr> nodes;
    Node *head = nullptr;
    Node *zero = nullptr;

  public:
    long size() const { return nodes.size(); }
    void push_back(const T &value);
    void mix();
    T calc_sum() const;

    template <typename U>
    friend std::ostream &operator<<(std::ostream &,
                                    const CircularLinkedList<U> &);
};

template <typename T>
void CircularLinkedList<T>::push_back(const T &value) {
    std::unique_ptr<Node> node_ptr = std::make_unique<Node>(value);
    Node *raw_ptr = node_ptr.get();
    if (size() == 0) {
        head = raw_ptr;
    } else {
        nodes.back()->insert_after(raw_ptr);
    }
    if (value == 0) {
        zero = raw_ptr;
    }
    nodes.emplace_back(std::move(node_ptr));
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const CircularLinkedList<T> &list) {
    auto *ptr = list.head;
    for (int i = 0; i < list.size(); ++i) {
        os << ptr->data;
        ptr = ptr->next;
        if (ptr != list.head) {
            os << ", ";
        }
    }
    return os;
}

template <typename T>
void CircularLinkedList<T>::mix() {
    if constexpr (aoc::DEBUG) {
        std::cerr << "initial arrangement:\n";
    }
    for (const std::unique_ptr<Node> &node : nodes) {
        if constexpr (aoc::DEBUG) {
            std::cerr << *this << "\n";
            std::cerr << "\nmoving " << node->data << ":\n";
        }
        if (node->data == 0) {
            continue;
        }
        auto *ptr = node.get();
        if (ptr == head) {
            head = ptr->next;
            if constexpr (aoc::DEBUG) {
                std::cerr << "(updated head to " << head->data
                          << " after unlinking)\n";
            }
        }
        node->unlink();
        int shift = node->data % (size() - 1);
        if constexpr (aoc::DEBUG) {
            std::cerr << "advancing by " << shift << "\n";
        }
        ptr = advance(ptr, shift);
        if (ptr->next == head && node->data > 0) {
            head = node.get();
            if constexpr (aoc::DEBUG) {
                std::cerr << "(updated head to " << head->data
                          << " after inserting)\n";
            }
        }
        if (shift > 0) {
            ptr->insert_after(node.get());
        } else {
            ptr->insert_before(node.get());
        }
    }
    if constexpr (aoc::DEBUG) {
        std::cerr << *this << "\n";
    }
}

template <typename T>
T CircularLinkedList<T>::calc_sum() const {
    T sum = 0;
    for (int i = 1; i <= 3; ++i) {
        Node *ptr = zero;
        ptr = advance(ptr, (i * 1000) % size());
        if constexpr (aoc::DEBUG) {
            std::cerr << i * 1000 << "th number: " << ptr->data << "\n";
        }
        sum += ptr->data;
    }
    return sum;
}

} // namespace aoc::day20

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day20;
    // need to store pointers here since the vector may change capacity
    CircularLinkedList<long> list_1;
    CircularLinkedList<long> list_2;
    // read file
    long value;
    while (infile >> value) {
        list_1.push_back(value);
        list_2.push_back(value * 811589153);
    }

    list_1.mix();
    std::cout << list_1.calc_sum() << "\n";

    for (int i = 0; i < 10; ++i) {
        list_2.mix();
        if constexpr (aoc::DEBUG) {
            std::cerr << "\n";
        }
    }
    std::cout << list_2.calc_sum() << "\n";

    return 0;
}
