/******************************************************************************
 * File:        day21.cpp
 *
 * Author:      yut23
 * Created:     2022-12-21
 *****************************************************************************/

#include "lib.h"
#include <cassert>   // for assert
#include <iostream>  // for cout, cerr
#include <map>       // for map
#include <memory>    // for shared_ptr, make_shared
#include <queue>     // for queue
#include <sstream>   // for istringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string, getline, stol

namespace aoc::day21 {

enum class Operation {
    add,
    subtract,
    multiply,
    divide,
};
std::istream &operator>>(std::istream &is, Operation &op) {
    char c;
    if (!(is >> c)) {
        return is;
    }
    switch (c) {
    case '+':
        op = Operation::add;
        break;
    case '-':
        op = Operation::subtract;
        break;
    case '*':
        op = Operation::multiply;
        break;
    case '/':
        op = Operation::divide;
        break;
    }
    return is;
}

struct Expression {
  public:
    virtual ~Expression();

    virtual bool has_value() const = 0;
    virtual long get_value() const = 0;
    virtual void equate(long value) = 0;
};

Expression::~Expression() {}

struct Integer : public Expression {
    long value;

    Integer() = delete;
    explicit Integer(long value) : value(value) {}
    ~Integer() = default;

    bool has_value() const override { return true; }
    long get_value() const override { return value; }
    void equate(long value) override { assert(this->value == value); }
};

std::ostream &operator<<(std::ostream &os, const Integer &integer) {
    os << integer.value;
    return os;
}

struct Unknown : public Expression {
  private:
    bool _has_value = false;
    long value = -1;

  public:
    Unknown() : Expression() {}
    ~Unknown() = default;

    bool has_value() const override { return _has_value; }
    long get_value() const override { return value; }
    void equate(long value) override {
        _has_value = true;
        this->value = value;
    }
};

using expr_ptr = std::shared_ptr<Expression>;

struct BinaryOp : public Expression {
    Operation op;
    expr_ptr lhs;
    expr_ptr rhs;

    BinaryOp(Operation op, expr_ptr lhs, expr_ptr rhs)
        : Expression(), op(op), lhs(lhs), rhs(rhs) {}
    ~BinaryOp() = default;

    bool has_value() const override { return false; }
    long get_value() const override { return 0; }
    void equate(long value) override;
};

void BinaryOp::equate(long value) {
    if (rhs->has_value()) {
        long rhs_value = rhs->get_value();
        long result;
        switch (op) {
        case Operation::add:
            // value = lhs + rhs
            // lhs = value - rhs
            result = value - rhs_value;
            break;
        case Operation::subtract:
            // value = lhs - rhs
            // lhs = value + rhs
            result = value + rhs_value;
            break;
        case Operation::multiply:
            // value = lhs * rhs
            // lhs = value / rhs
            result = value / rhs_value;
            break;
        case Operation::divide:
            // value = lhs / rhs
            // lhs = value * rhs
            result = value * rhs_value;
            break;
        }
        lhs->equate(result);
    } else if (lhs->has_value()) {
        long lhs_value = lhs->get_value();
        long result;
        switch (op) {
        case Operation::add:
            // value = lhs + rhs
            // rhs = value - lhs
            result = value - lhs_value;
            break;
        case Operation::subtract:
            // value = lhs - rhs
            // rhs + value = lhs
            // rhs = lhs - value
            result = lhs_value - value;
            break;
        case Operation::multiply:
            // value = lhs * rhs
            // rhs = value / lhs
            result = value / lhs_value;
            break;
        case Operation::divide:
            // value = lhs / rhs
            // rhs * value = lhs
            // rhs = lhs / value
            result = lhs_value / value;
            break;
        }
        rhs->equate(result);
    } else {
        assert(false);
    }
}

expr_ptr add(expr_ptr lhs, expr_ptr rhs) {
    if (lhs->has_value() && rhs->has_value()) {
        return std::make_shared<Integer>(lhs->get_value() + rhs->get_value());
    }
    return std::make_shared<BinaryOp>(Operation::add, lhs, rhs);
}
expr_ptr subtract(expr_ptr lhs, expr_ptr rhs) {
    if (lhs->has_value() && rhs->has_value()) {
        return std::make_shared<Integer>(lhs->get_value() - rhs->get_value());
    }
    return std::make_shared<BinaryOp>(Operation::subtract, lhs, rhs);
}
expr_ptr multiply(expr_ptr lhs, expr_ptr rhs) {
    if (lhs->has_value() && rhs->has_value()) {
        return std::make_shared<Integer>(lhs->get_value() * rhs->get_value());
    }
    return std::make_shared<BinaryOp>(Operation::multiply, lhs, rhs);
}
expr_ptr divide(auto lhs, auto rhs) {
    if (lhs->has_value() && rhs->has_value()) {
        return std::make_shared<Integer>(lhs->get_value() / rhs->get_value());
    }
    return std::make_shared<BinaryOp>(Operation::divide, lhs, rhs);
}

struct Monkey {
    std::string name{};
    bool done = false;
    expr_ptr number;

    char op;
    std::string lhs_name;
    std::string rhs_name;

    void evaluate(const std::map<std::string, expr_ptr> &numbers) {
        if (done) {
            return;
        }
        auto lhs_it = numbers.find(lhs_name), rhs_it = numbers.find(rhs_name);
        if (lhs_it == numbers.end() || rhs_it == numbers.end()) {
            return;
        }
        // both operands are present
        if constexpr (aoc::DEBUG) {
            std::cerr << "evaluating monkey " << name << ": " << lhs_name << " "
                      << op << " " << rhs_name << "\n";
        }
        expr_ptr lhs = lhs_it->second, rhs = rhs_it->second;
        switch (op) {
        case '+':
            number = add(lhs, rhs);
            break;
        case '-':
            number = subtract(lhs, rhs);
            break;
        case '*':
            number = multiply(lhs, rhs);
            break;
        case '/':
            number = divide(lhs, rhs);
            break;
        case '=':
            if (!lhs->has_value()) {
                assert(rhs->has_value());
                lhs->equate(rhs->get_value());
            } else if (!rhs->has_value()) {
                assert(lhs->has_value());
                rhs->equate(lhs->get_value());
            } else {
                assert(false);
            }
            number = std::make_shared<Integer>(1);
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
        monkey->number = std::make_shared<Integer>(std::stol(job));
        monkey->done = true;
    } catch (std::invalid_argument) {
        std::istringstream ss{job};
        ss >> monkey->lhs_name >> monkey->op >> monkey->rhs_name;
    }
    return monkey;
}

void part_1(std::istream &is) {
    std::queue<Monkey *> pending_monkeys;
    Monkey *monkey;
    while ((monkey = read_monkey(is)) != nullptr) {
        pending_monkeys.push(monkey);
    }

    std::map<std::string, expr_ptr> numbers{};
    while (!pending_monkeys.empty()) {
        monkey = pending_monkeys.front();
        pending_monkeys.pop();
        if (!monkey->done) {
            monkey->evaluate(numbers);
        }
        if (monkey->done) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "got value for " << monkey->name << ": "
                          << monkey->number->get_value() << "\n";
            }
            numbers[monkey->name] = monkey->number;
            delete monkey;
        } else {
            pending_monkeys.push(monkey);
        }
    }

    std::cout << numbers["root"]->get_value() << std::endl;
}

void part_2(std::istream &is) {
    std::queue<Monkey *> pending_monkeys;
    Monkey *monkey;
    while ((monkey = read_monkey(is)) != nullptr) {
        pending_monkeys.push(monkey);
    }

    expr_ptr humn = std::make_shared<Unknown>();
    std::map<std::string, expr_ptr> numbers{};
    while (!pending_monkeys.empty()) {
        monkey = pending_monkeys.front();
        pending_monkeys.pop();
        if (monkey->name == "root") {
            monkey->op = '=';
        } else if (monkey->name == "humn") {
            monkey->number = humn;
        }
        if (!monkey->done) {
            monkey->evaluate(numbers);
        }
        if (monkey->done) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "got value for " << monkey->name << ": "
                          << monkey->number->get_value() << "\n";
            }
            numbers[monkey->name] = monkey->number;
            delete monkey;
        } else {
            pending_monkeys.push(monkey);
        }
    }

    assert(humn->has_value());
    std::cout << humn->get_value() << std::endl;
}

} // namespace aoc::day21

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);
    aoc::day21::part_1(infile);
    infile.clear();
    infile.seekg(0);
    aoc::day21::part_2(infile);
    return 0;
}
