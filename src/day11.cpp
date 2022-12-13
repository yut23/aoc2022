/******************************************************************************
 * File:        day11.cpp
 *
 * Author:      yut23
 * Created:     2022-12-11
 *****************************************************************************/

#include "lib.h"
#include <algorithm>  // for ranges::transform, ranges::sort
#include <cassert>    // for assert
#include <cstddef>    // for size_t
#include <deque>      // for deque
#include <functional> // for function, greater
#include <iostream>   // for cout, cerr
#include <iterator>   // for back_inserter
#include <numeric>    // for lcm, transform_reduce
#include <sstream>    // for stringstream
#include <string>     // for string, getline, stoi
#include <utility>    // for move
#include <vector>     // for vector

namespace aoc::day11 {

constexpr bool verbose = aoc::DEBUG && false;

using item_t = unsigned long;

class Monkey {
    std::deque<item_t> items{};
    std::function<item_t(item_t)> operation{};
    int true_dest{};
    int false_dest{};

  public:
    item_t divisor{};
    int inspect_count = 0;

    template <bool part_1>
    void process_items(std::vector<Monkey> &monkeys, item_t modulus);
    void recieve_item(item_t item) { items.push_back(item); }

    friend std::istream &operator>>(std::istream &, Monkey &);
    friend std::ostream &operator<<(std::ostream &,
                                    const std::vector<Monkey> &);
};

template <bool part_1>
void Monkey::process_items(std::vector<Monkey> &monkeys, item_t modulus) {
    for (; !items.empty(); items.pop_front(), ++inspect_count) {
        item_t worry_level = items.front();
        if constexpr (verbose && part_1) {
            std::cerr << "  Monkey inspects an item with a worry level of "
                      << worry_level << "." << std::endl;
            std::cerr << "    Worry level changes from " << worry_level
                      << " to ";
        }
        worry_level = operation(worry_level);
        if constexpr (verbose && part_1) {
            std::cerr << worry_level << "." << std::endl;
        }
        if constexpr (part_1) {
            worry_level /= 3;
            if constexpr (verbose) {
                std::cerr << "    Monkey gets bored with item. Worry level is "
                             "divided by 3 to "
                          << worry_level << "." << std::endl;
            }
        } else {
            worry_level %= modulus;
        }
        if constexpr (verbose && part_1) {
            std::cerr << "    Current worry level is ";
        }
        int dest_monkey;
        if (worry_level % divisor == 0) {
            dest_monkey = true_dest;
        } else {
            dest_monkey = false_dest;
            if constexpr (verbose && part_1) {
                std::cerr << "not ";
            }
        }
        if constexpr (verbose && part_1) {
            std::cerr << "divisible by " << divisor << "." << std::endl;
        }
        monkeys.at(dest_monkey).recieve_item(worry_level);
        if constexpr (verbose && part_1) {
            std::cerr << "    Item with worry level " << worry_level
                      << " is thrown to monkey " << dest_monkey << "."
                      << std::endl;
        }
    }
}

std::istream &operator>>(std::istream &is, Monkey &m) {
    std::string line;

    // skip any leading whitespace and first line
    if (!std::getline(is >> std::ws, line)) {
        return is;
    }
    assert(line.starts_with("Monkey "));

    // read starting items line
    std::getline(is, line);
    assert(line.starts_with("  Starting items:"));
    std::istringstream ss{line};
    // skip first two words: "Starting items:"
    ss >> aoc::skip(2);
    while (true) {
        item_t item;
        char comma;
        assert(ss >> item);
        m.recieve_item(item);
        // try reading the comma before the next item
        if (!(ss >> comma)) {
            break;
        }
    }

    // read operation line: "Operation: new = old (operator) (operand)"
    char op;
    std::string operand;
    is >> aoc::skip(4) >> op >> operand;
    if (operand == "old") {
        assert(op == '*');
        m.operation = [](item_t old) { return old * old; };
    } else {
        int value = std::stoi(operand);
        switch (op) {
        case '+':
            m.operation = [=](item_t old) { return old + value; };
            break;
        case '*':
            m.operation = [=](item_t old) { return old * value; };
            break;
        default:
            assert(false);
        }
    }

    // read test line: "Test: divisible by (divisor)"
    is >> aoc::skip(3) >> m.divisor;

    // read true line: "If true: throw to monkey (true_dest)"
    is >> aoc::skip(5) >> m.true_dest;

    // read false line: "If false: throw to monkey (false_dest)"
    is >> aoc::skip(5) >> m.false_dest;

    return is;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Monkey> &monkeys) {
    for (std::size_t i = 0; i < monkeys.size(); ++i) {
        os << "Monkey " << i << ": ";
        const Monkey &monkey = monkeys[i];
        for (auto item_it = monkey.items.cbegin();
             item_it != monkey.items.cend(); ++item_it) {
            if (item_it != monkey.items.cbegin()) {
                os << ", ";
            }
            os << *item_it;
        }
        os << std::endl;
    }
    return os;
}

template <bool part_1>
item_t do_monkey_business(std::vector<Monkey> monkeys, int num_rounds,
                          item_t modulus = 0) {
    for (int round = 1; round <= num_rounds; ++round) {
        for (std::size_t i = 0; i < monkeys.size(); ++i) {
            if constexpr (verbose && part_1) {
                std::cerr << "Monkey " << i << ":" << std::endl;
            }
            monkeys.at(i).process_items<part_1>(monkeys, modulus);
        }
        if constexpr (aoc::DEBUG && part_1) {
            std::cerr << "After round " << round
                      << ", the monkeys are holding items with these worry "
                         "levels:"
                      << std::endl;
            std::cerr << monkeys << std::endl;
        }
        if constexpr (aoc::DEBUG) {
            if (round == 1 || round == 20 || round % 1000 == 0) {
                std::cerr << "== After round " << round << " ==" << std::endl;
                for (std::size_t i = 0; i < monkeys.size(); ++i) {
                    std::cerr << "Monkey " << i << " inspected items "
                              << monkeys[i].inspect_count << " times."
                              << std::endl;
                }
                std::cerr << std::endl;
            }
        }
    }

    std::vector<item_t> inspect_counts{};
    std::ranges::transform(monkeys, std::back_inserter(inspect_counts),
                           [](const Monkey &m) { return m.inspect_count; });
    std::ranges::sort(inspect_counts, std::greater{});
    return inspect_counts[0] * inspect_counts[1];
}

} // namespace aoc::day11

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day11;
    std::vector<Monkey> monkeys{};
    while (true) {
        Monkey monkey;
        if (!(infile >> monkey)) {
            break;
        }
        monkeys.push_back(std::move(monkey));
    }
    if constexpr (verbose) {
        std::cerr << monkeys << std::endl;
    }

    // part 1
    std::cout << do_monkey_business<true>(monkeys, 20) << std::endl;

    // find the LCM of all the monkeys' divisors
    item_t modulus = std::transform_reduce(
        monkeys.cbegin(), monkeys.cend(), 1, std::lcm<item_t, item_t>,
        [](const Monkey &m) { return m.divisor; });

    // part 2
    std::cout << do_monkey_business<false>(monkeys, 10000, modulus)
              << std::endl;

    return 0;
}
