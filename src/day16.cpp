/******************************************************************************
 * File:        day16.cpp
 *
 * Author:      yut23
 * Created:     2022-12-18
 *****************************************************************************/

#include "lib.h"
#include <algorithm>     // for find_if, for_each, find
#include <cassert>       // for assert
#include <compare>       // for weak_ordering
#include <iostream>      // for cout, cerr
#include <limits>        // for numeric_limits
#include <map>           // for map
#include <memory>        // for unique_ptr, make_unique
#include <regex>         // for regex, smatch, regex_search, sregex_iterator
#include <string>        // for string, getline
#include <unordered_set> // for unordered_set
#include <vector>        // for vector

namespace aoc::day16 {

using Key = std::string;

struct Valve;

struct Tunnel {
    int length;
    Valve *valve;

    Tunnel(int length, Valve *valve) : length(length), valve(valve) {}

    Valve &operator*() { return *valve; }
    const Valve &operator*() const { return *valve; }
    Valve *operator->() { return valve; }
    const Valve *operator->() const { return valve; }
};

struct Valve {
    int flow_rate{};
    Key name{};
    std::vector<Tunnel> tunnels{};

    explicit Valve(const Key &name) : flow_rate(-1), name(name) {}
    Valve(int flow_rate, const Key &name) : flow_rate(flow_rate), name(name) {}

    std::vector<Tunnel>::iterator get_tunnel_to(const Valve *);
};

std::vector<Tunnel>::iterator Valve::get_tunnel_to(const Valve *other) {
    return std::find_if(
        tunnels.begin(), tunnels.end(),
        [=](const Tunnel &tunnel) { return tunnel.valve == other; });
}

struct Graph {
    std::map<Key, std::unique_ptr<Valve>> valves;

    Valve *get_valve(const Key &name) {
        auto it = valves.find(name);
        if (it == valves.end()) {
            it = valves.emplace(name, std::make_unique<Valve>(name)).first;
        }
        return it->second.get();
    }

    // collapse valves with zero flow rate
    void simplify();

    void read_line(const std::string &);
    void output_graphviz(std::ostream &);
};

void Graph::read_line(const std::string &line) {
    static std::regex number_regex{"\\d+"};
    static std::regex valve_regex{"[A-Z]{2}"};

    std::smatch number_match;
    assert(std::regex_search(line, number_match, number_regex));
    int flow_rate = std::stoi(number_match[0]);
    auto valve_iter =
        std::sregex_iterator(line.cbegin(), line.cend(), valve_regex);
    auto valves_end = std::sregex_iterator();
    Key name = (valve_iter++)->str();
    Valve *valve = get_valve(name);
    assert(valve->flow_rate == -1);
    valve->flow_rate = flow_rate;
    for (; valve_iter != valves_end; ++valve_iter) {
        valve->tunnels.emplace_back(1, get_valve(valve_iter->str()));
    }
}

void Graph::output_graphviz(std::ostream &os) {
    os << "strict graph {\n  overlap=\"scale\"\n";
    for (const auto &[name, valve] : valves) {
        if (valve->flow_rate > 0) {
            os << "  " << name << " [label=\"" << name << " ("
               << valve->flow_rate << ")\", color=blue]\n";
        }
        for (const Tunnel &neighbor : valve->tunnels) {
            os << "  " << name << " -- " << neighbor->name;
            if (neighbor.length > 1) {
                os << " [label=\"" << neighbor.length << "\"]";
            }
            os << "\n";
        }
    }
    os << "}\n";
}

void Graph::simplify() {
    // loop over all nodes and collapse the ones with zero flow (except for AA)
    std::unordered_set<Key> to_delete{};
    for (auto &[name, valve_ptr] : valves) {
        Valve *valve = valve_ptr.get();
        if (valve->name == "AA" || valve->flow_rate > 0) {
            continue;
        }
        if (valve->tunnels.size() != 2) {
            // ignore branching valves for now
            continue;
        }
        const Tunnel &tunnel_1 = valve->tunnels[0];
        const Tunnel &tunnel_2 = valve->tunnels[1];
        int new_length = tunnel_1.length + tunnel_2.length;
        // relink valve connected to tunnel 1 to valve connected to tunnel 2
        auto tunnel_1_reverse = tunnel_1.valve->get_tunnel_to(valve);
        tunnel_1_reverse->length = new_length;
        tunnel_1_reverse->valve = tunnel_2.valve;
        // relink valve connected to tunnel 2 to valve connected to tunnel 1
        auto tunnel_2_reverse = tunnel_2.valve->get_tunnel_to(valve);
        tunnel_2_reverse->length = new_length;
        tunnel_2_reverse->valve = tunnel_1.valve;
        // mark this valve to be deleted
        to_delete.emplace(name);
    }
    std::ranges::for_each(to_delete,
                          [=](const Key &name) { valves.erase(name); });
}

using DistanceMap = std::map<Key, std::map<Key, int>>;

template <typename T>
DistanceMap floyd_warshall(const T &valves) {
    // follows https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
    DistanceMap dists{};
    std::vector<Key> keys{};
    for (const auto &[u, valve] : valves) {
        keys.push_back(u);
        for (const auto &[v, _] : valves) {
            // initialize all distances to a very large number (but not too big,
            // as we don't want to overflow)
            dists[u][v] = std::numeric_limits<int>::max() / 10;
        }
        dists[u][u] = 0;
        for (const Tunnel &tunnel : valve->tunnels) {
            Key v = tunnel->name;
            dists[u][v] = tunnel.length;
        }
    }
    // standard implementation
    for (const Key &k : keys) {
        for (const Key &i : keys) {
            for (const Key &j : keys) {
                if (dists[i][j] > dists[i][k] + dists[k][j]) {
                    dists[i][j] = dists[i][k] + dists[k][j];
                }
            }
        }
    }
    return dists;
}

class GreedySolver {
    const Graph &graph;
    const DistanceMap &dists;
    std::vector<Key> opened_valves{};

  public:
    GreedySolver(const Graph &graph, const DistanceMap &dists)
        : graph(graph), dists(dists) {}

    int solve(const Key &current_valve, int remaining_time,
              std::vector<Key> opened_valves = {});
};

struct Move {
    Key valve{};
    int time_taken = 0;
    int future_value = 0;

    std::weak_ordering operator<=>(const Move &rhs) const {
        return future_value <=> rhs.future_value;
    }
};

int GreedySolver::solve(const Key &current_valve, int remaining_time,
                        std::vector<Key> opened_valves) {
    Move best_move{};
    for (const auto &[name, valve] : graph.valves) {
        if (std::ranges::find(opened_valves, name) != opened_valves.end()) {
            // skip valves we've already opened
            continue;
        }
        // calculate total future value for an unopened valve, given the
        // distance to the valve and the remaining time
        int distance = dists.at(current_valve).at(name);
        assert(distance >= 0);
        // deduct the travel time plus the minute it takes to open the valve
        int future_value = (remaining_time - (distance + 1)) * valve->flow_rate;
        if (future_value > 0 && future_value > best_move.future_value) {
            best_move.valve = name;
            best_move.time_taken = distance + 1;
            best_move.future_value = future_value;
        }
    }
    if (best_move.future_value == 0) {
        // base case
        return 0;
    }

    // make move
    if constexpr (aoc::DEBUG) {
        std::cerr << "moving " << best_move.time_taken - 1 << " spaces to "
                  << best_move.valve << " and opening it at minute "
                  << (30 - remaining_time) + best_move.time_taken
                  << ", to release " << best_move.future_value
                  << " total pressure at a flow rate of "
                  << graph.valves.at(best_move.valve)->flow_rate << "\n";
    }
    opened_valves.push_back(best_move.valve);
    return best_move.future_value + solve(best_move.valve,
                                          remaining_time - best_move.time_taken,
                                          opened_valves);
}

} // namespace aoc::day16

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day16;
    Graph graph{};
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        graph.read_line(line);
    }
    graph.simplify();
    // graph.output_graphviz(std::cout);

    auto dists = floyd_warshall(graph.valves);
    GreedySolver solver{graph, dists};
    if constexpr (aoc::DEBUG) {
        std::cerr << "starting at AA\n";
    }
    int part_1 = solver.solve("AA", 30);
    std::cout << part_1 << std::endl;
    return 0;
}
