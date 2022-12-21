/******************************************************************************
 * File:        day21.cpp
 *
 * Author:      yut23
 * Created:     2022-12-21
 *****************************************************************************/

#include "lib.h"
#include <iostream>  // for cout
#include <map>       // for map
#include <queue>     // for queue
#include <sstream>   // for istringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string, getline, stoi

namespace aoc::day21 {

struct Monkey {
    std::string name{};
    std::string job{};
    bool done = false;
    long number = 0;

    std::string dep_1;
    std::string dep_2;
    char op;

    void evaluate(const std::map<std::string, long> &numbers) {
        if (done) {
            return;
        }
        auto it_1 = numbers.find(dep_1), it_2 = numbers.find(dep_2);
        if (it_1 == numbers.end() || it_2 == numbers.end()) {
            return;
        }
        // both operands are present
        long num_1 = it_1->second, num_2 = it_2->second;
        switch (op) {
        case '+':
            number = num_1 + num_2;
            break;
        case '-':
            number = num_1 - num_2;
            break;
        case '*':
            number = num_1 * num_2;
            break;
        case '/':
            number = num_1 / num_2;
            break;
        }
        done = true;
    }
};

Monkey *read_monkey(std::istream &is) {
    Monkey *monkey = new Monkey();
    if (!std::getline(is, monkey->name, ':')) {
        delete monkey;
        return nullptr;
    }
    std::string job;
    std::getline(is, job);
    try {
        monkey->number = std::stoi(job);
        monkey->done = true;
    } catch (std::invalid_argument) {
        std::istringstream ss{job};
        ss >> monkey->dep_1 >> monkey->op >> monkey->dep_2;
    }
    return monkey;
}

} // namespace aoc::day21

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day21;
    std::queue<Monkey *> pending_monkeys;
    Monkey *monkey;
    while ((monkey = read_monkey(infile)) != nullptr) {
        pending_monkeys.push(monkey);
    }

    std::map<std::string, long> numbers{};
    while (!pending_monkeys.empty()) {
        monkey = pending_monkeys.front();
        pending_monkeys.pop();
        if (!monkey->done) {
            monkey->evaluate(numbers);
        }
        if (monkey->done) {
            numbers[monkey->name] = monkey->number;
            delete monkey;
        } else {
            pending_monkeys.push(monkey);
        }
    }

    std::cout << numbers["root"] << std::endl;
    return 0;
}
