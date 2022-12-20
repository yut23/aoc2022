/******************************************************************************
 * File:        day16.cpp
 *
 * Author:      yut23
 * Created:     2022-12-18
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for find_if
#include <cassert>   // for assert
#include <iostream>  // for cout, cerr
#include <limits>    // for numeric_limits
#include <map>       // for map
#include <memory>    // for unique_ptr, make_unique
#include <regex>     // for regex, smatch, regex_search, sregex_iterator
#include <string>    // for string, getline
#include <vector>    // for vector

namespace aoc::day16 {

using Key = unsigned int;

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
    std::string name{};
    Key key{};
    std::vector<Tunnel> tunnels{};

    explicit Valve(const std::string &name, Key key)
        : flow_rate(-1), name(name), key(key) {}
    Valve(int flow_rate, const std::string &name, Key key)
        : flow_rate(flow_rate), name(name), key(key) {}

    std::vector<Tunnel>::iterator get_tunnel_to(const Valve *);
};

std::vector<Tunnel>::iterator Valve::get_tunnel_to(const Valve *other) {
    return std::find_if(
        tunnels.begin(), tunnels.end(),
        [=](const Tunnel &tunnel) { return tunnel.valve == other; });
}

struct Graph {
    std::map<std::string, Key> name_lookup;
    std::vector<std::unique_ptr<Valve>> valves;

    Valve *get_valve_by_name(const std::string &name) {
        auto it = name_lookup.find(name);
        if (it == name_lookup.end()) {
            it = name_lookup.emplace(name, valves.size()).first;
            valves.emplace_back(std::make_unique<Valve>(name, valves.size()));
        }
        return valves[it->second].get();
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
    Valve *valve = get_valve_by_name(valve_iter->str());
    ++valve_iter;
    assert(valve->flow_rate == -1);
    valve->flow_rate = flow_rate;
    for (; valve_iter != valves_end; ++valve_iter) {
        valve->tunnels.emplace_back(1, get_valve_by_name(valve_iter->str()));
    }
}

void Graph::output_graphviz(std::ostream &os) {
    os << "strict graph {\n  overlap=\"scale\"\n";
    for (Key key = 0; key < valves.size(); ++key) {
        Valve *valve = valves[key].get();
        os << "  " << key << " [label=\"" << valve->name;
        if (valve->flow_rate > 0) {
            os << " (" << valve->flow_rate << ")\", color=blue]\n";
        } else {
            os << "\"]\n";
        }
        for (const Tunnel &neighbor : valve->tunnels) {
            os << "  " << key << " -- " << neighbor->key;
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
    for (Key key = valves.size() - 1; key-- != 0;) {
        Valve *valve = valves[key].get();
        if (valve->name == "AA" || valve->flow_rate > 0) {
            continue;
        }
        if (valve->tunnels.size() != 2) {
            // ignore branching valves
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
        name_lookup.erase(valve->name);
        valves.erase(valves.begin() + key);
    }
    // renumber the remaining valves
    for (Key new_key = 0; new_key < valves.size(); ++new_key) {
        Valve *valve = valves[new_key].get();
        valve->key = new_key;
        name_lookup[valve->name] = new_key;
    }
}

using DistanceMap = std::vector<std::vector<int>>;

template <typename T>
DistanceMap floyd_warshall(const T &valves) {
    // follows https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
    DistanceMap dists(valves.size());
    for (Key u = 0; u < valves.size(); ++u) {
        // initialize all distances to a very large number (but not too big, as
        // we don't want to overflow)
        dists[u].resize(valves.size(), std::numeric_limits<int>::max() / 10);
        dists[u][u] = 0;
        for (const Tunnel &tunnel : valves[u]->tunnels) {
            Key v = tunnel->key;
            dists[u][v] = tunnel.length;
        }
    }
    // standard implementation
    for (Key k = 0; k < valves.size(); ++k) {
        for (Key i = 0; i < valves.size(); ++i) {
            for (Key j = 0; j < valves.size(); ++j) {
                if (dists[i][j] > dists[i][k] + dists[k][j]) {
                    dists[i][j] = dists[i][k] + dists[k][j];
                }
            }
        }
    }
    return dists;
}

class DFSSolver {
    const Graph &graph;
    const DistanceMap &dists;

  public:
    DFSSolver(const Graph &graph, const DistanceMap &dists)
        : graph(graph), dists(dists) {}

    int solve(Key current_key, int remaining_time,
              unsigned int visited_valves = 0);
};

int DFSSolver::solve(Key current_key, int remaining_time,
                     unsigned int visited_valves) {
    int best_total = 0;
    const auto &distances = dists[current_key];
    for (Key key = 0, mask = 1; key < graph.valves.size(); ++key, mask <<= 1) {
        Valve *valve = graph.valves[key].get();
        if (visited_valves & mask || valve->flow_rate == 0) {
            // skip valves we've already opened
            continue;
        }
        // calculate total future value for an unopened valve, given the
        // distance to the valve and the remaining time
        int distance = distances[key];
        assert(distance >= 0);
        // deduct the travel time plus the minute it takes to open the valve
        int future_value = (remaining_time - (distance + 1)) * valve->flow_rate;
        if (future_value <= 0) {
            continue;
        }
        // recurse from the new state
        int total = future_value + solve(key, remaining_time - (distance + 1),
                                         visited_valves | mask);
        if (total > best_total) {
            best_total = total;
        }
    }
    return best_total;
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
    DFSSolver solver{graph, dists};
    int part_1 = solver.solve(graph.name_lookup.at("AA"), 30);
    std::cout << part_1 << std::endl;
    return 0;
}
