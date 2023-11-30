/******************************************************************************
 * File:        day16.cpp
 *
 * Author:      yut23
 * Created:     2022-12-18
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for find_if, min
#include <array>     // for array
#include <cassert>   // for assert
#include <iostream>  // for cout, cerr
#include <limits>    // for numeric_limits
#include <map>       // for map
#include <memory>    // for unique_ptr, make_unique
#include <regex>     // for regex, smatch, regex_search, sregex_iterator
#include <string>    // for string, getline, stoi
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

struct SolverInfo {
    const Graph &graph;
    const DistanceMap &dists;
    const Key &initial_pos;

    SolverInfo(const Graph &graph, const DistanceMap &dists)
        : graph(graph), dists(dists), initial_pos(graph.name_lookup.at("AA")) {}
};

class DFSSolver {
    const SolverInfo &info;
    Key my_pos;
    int my_remaining_time;
    Key elephant_pos;
    int elephant_remaining_time;

  public:
    DFSSolver(const SolverInfo &info, int my_time, int elephant_time = 0)
        : info(info), my_pos(info.initial_pos), my_remaining_time(my_time),
          elephant_pos(info.initial_pos),
          elephant_remaining_time(elephant_time) {}

    int solve(unsigned int visited_valves = 0, int depth = 0);
};

int DFSSolver::solve(unsigned int visited_valves, int depth) {
    // move the one with more remaining time
    const bool move_me = my_remaining_time >= elephant_remaining_time;
    Key &current_pos = move_me ? my_pos : elephant_pos;
    int &remaining_time = move_me ? my_remaining_time : elephant_remaining_time;
    if constexpr (aoc::DEBUG) {
        std::cerr << std::string(depth * 2, ' ') << "moving "
                  << (move_me ? "me" : "elephant") << " to "
                  << info.graph.valves[current_pos]->name << " at time "
                  << (30 - remaining_time) << "\n";
    }
    const auto &distances = info.dists[current_pos];
    Key prev_pos = current_pos;
    int best_total = 0;
    for (Key key = 0, mask = 1; key < info.graph.valves.size();
         ++key, mask <<= 1) {
        Valve *valve = info.graph.valves[key].get();
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
        current_pos = key;
        remaining_time -= distance + 1;
        int total = future_value + solve(visited_valves | mask, depth + 1);
        remaining_time += distance + 1;
        current_pos = prev_pos;
        if (total > best_total) {
            best_total = total;
        }
    }
    if constexpr (aoc::DEBUG) {
        if (best_total > 0) {
            std::cerr << std::string(depth * 2, ' ')
                      << "best total pressure: " << best_total << "\n";
        }
    }
    return best_total;
}

struct Entity {
    Key pos;
    int travel_time;

    Entity() : pos(-1), travel_time(-1) {}

    Entity(Key pos, int travel_time) : pos(pos), travel_time(travel_time) {}

    Entity travel() const {
        // assert(travel_time > 0);
        return Entity(pos, travel_time - 1);
    }
};

struct State {
    const Entity entity_1;
    const Entity entity_2;
    const int total_flow;
    const unsigned int visited_valves;
    bool good = true;

    State(const SolverInfo &info, int total_time, bool use_entity_2)
        : entity_1(info.initial_pos, 0),
          entity_2(info.initial_pos, use_entity_2 ? 0 : total_time),
          total_flow(0), visited_valves(0) {}
    State(int total_flow, unsigned int visited_valves, Entity entity_1,
          Entity entity_2)
        : entity_1(entity_1), entity_2(entity_2), total_flow(total_flow),
          visited_valves(visited_valves) {}

    State wait() const {
        assert(entity_1.travel_time > 0);
        assert(entity_2.travel_time > 0);
        return State(total_flow, visited_valves, entity_1.travel(),
                     entity_2.travel());
    }
    template <int index>
    State move_one(const Entity &moved_entity, int future_value) const {
        static_assert(index == 1 || index == 2, "index must be 1 or 2");
        if constexpr (index == 1) {
            assert(entity_1.travel_time == 0 && entity_2.travel_time > 0);
        } else {
            assert(entity_1.travel_time > 0 && entity_2.travel_time == 0);
        }
        assert(future_value > 0);
        if constexpr (index == 1) {
            return State(total_flow + future_value,
                         visited_valves | (1 << moved_entity.pos), moved_entity,
                         entity_2.travel());
        } else {
            return State(total_flow + future_value,
                         visited_valves | (1 << moved_entity.pos),
                         entity_1.travel(), moved_entity);
        }
    }

    State move_both(const Entity &new_entity_1, const Entity &new_entity_2,
                    int future_value) const {
        assert(entity_1.travel_time == 0 && entity_2.travel_time == 0);
        assert(future_value > 0);
        return State(total_flow + future_value,
                     visited_valves | (1 << new_entity_1.pos) |
                         (1 << new_entity_2.pos),
                     new_entity_1, new_entity_2);
    }

    int flow_upper_bound(const SolverInfo &info, int remaining_time) {
        int max_flow = total_flow;
        const auto &distances_1 = info.dists[entity_1.pos];
        const auto &distances_2 = info.dists[entity_2.pos];
        for (Key key = 0; key < info.graph.valves.size(); ++key) {
            if (visited_valves & (1 << key)) {
                continue;
            }
            int distance_1 = distances_1[key] + entity_1.travel_time;
            int distance_2 = distances_2[key] + entity_2.travel_time;
            int future_value =
                (remaining_time - (std::min(distance_1, distance_2) + 1)) *
                info.graph.valves[key]->flow_rate;
            if (future_value <= 0) {
                continue;
            }
            max_flow += future_value;
        }
        return max_flow;
    }
};

int solve_bfs(const SolverInfo &info, int total_time,
              bool use_entity_2 = false) {
    std::vector<State> curr_queue{{State(info, total_time, use_entity_2)}};
    std::vector<State> next_queue{};

    int best_total = 0;
    for (int remaining_time = total_time; remaining_time > 0;
         --remaining_time) {
        int state_count = 0;
        int best_actual_flow = 0;
        for (auto it = curr_queue.begin(); it != curr_queue.end(); ++it) {
            const State &state = *it;
            if (!state.good) {
                continue;
            }
            if (state.total_flow > best_total) {
                best_total = state.total_flow;
            }
            if (state.entity_1.travel_time > 0 &&
                state.entity_2.travel_time > 0) {
                next_queue.push_back(state.wait());
                if (next_queue.back().total_flow > best_actual_flow) {
                    best_actual_flow = next_queue.back().total_flow;
                }
                continue;
            }
            ++state_count;
            const auto &distances_1 = info.dists[state.entity_1.pos];
            const auto &distances_2 = info.dists[state.entity_2.pos];
            for (Key key_a = 0; key_a < info.graph.valves.size(); ++key_a) {
                int flow_rate = info.graph.valves[key_a]->flow_rate;
                if (state.visited_valves & (1 << key_a) || flow_rate == 0) {
                    // skip valves we've already opened
                    continue;
                }
                // calculate total future value for an unopened valve, given the
                // distance to the valve and the remaining time
                int distance_a;
                if (state.entity_1.travel_time == 0) {
                    distance_a = distances_1[key_a];
                } else {
                    distance_a = distances_2[key_a];
                }
                // deduct the travel time plus the minute it takes to open the
                // valve
                int future_value_a =
                    (remaining_time - (distance_a + 1)) * flow_rate;
                if (future_value_a <= 0) {
                    continue;
                }
                if (state.entity_2.travel_time > 0) {
                    // move entity 1
                    next_queue.push_back(state.move_one<1>(
                        Entity(key_a, distance_a), future_value_a));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                    continue;
                }
                if (state.entity_1.travel_time > 0) {
                    // move entity 2
                    next_queue.push_back(state.move_one<2>(
                        Entity(key_a, distance_a), future_value_a));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                    continue;
                }
                // loop over possible destinations for entity 2
                assert(state.entity_1.travel_time == 0 &&
                       state.entity_2.travel_time == 0);
                for (Key key_b = 0; key_b < info.graph.valves.size(); ++key_b) {
                    if (key_b == key_a) {
                        continue;
                    }
                    flow_rate = info.graph.valves[key_b]->flow_rate;
                    if (state.visited_valves & (1 << key_b) || flow_rate == 0) {
                        // skip valves we've already opened
                        continue;
                    }
                    // calculate total future value for an unopened valve, given
                    // the distance to the valve and the remaining time
                    int distance_b = distances_2[key_b];
                    // deduct the travel time plus the minute it takes to open
                    // the valve
                    int future_value_b =
                        (remaining_time - (distance_b + 1)) * flow_rate;
                    if (future_value_b <= 0) {
                        continue;
                    }
                    next_queue.push_back(state.move_both(
                        Entity(key_a, distance_a), Entity(key_b, distance_b),
                        future_value_a + future_value_b));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                }
            }
        }
        int reject_count = 0;
        for (auto it = next_queue.begin(); it != next_queue.end(); ++it) {
            if (it->flow_upper_bound(info, remaining_time - 1) <
                best_actual_flow) {
                it->good = false;
                ++reject_count;
            }
        }
        if constexpr (aoc::DEBUG) {
            std::cerr << "remaining time = " << total_time - remaining_time + 1
                      << ": " << state_count << " states, "
                      << next_queue.size() - reject_count << " branches ("
                      << reject_count << " rejected)\n";
        }
        curr_queue.clear();
        std::swap(curr_queue, next_queue);
    }
    return best_total;
}

namespace detail {
template <typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)>
create_array(T value, std::index_sequence<Is...>) {
    // cast Is to void to remove the warning: unused value
    return {{(static_cast<void>(Is), value)...}};
}
} // namespace detail

template <std::size_t N, typename T>
constexpr std::array<T, N> create_array(const T &value) {
    return detail::create_array(value, std::make_index_sequence<N>());
}

template <std::size_t N>
struct State2 {
    static constexpr std::size_t size = N;
    const std::array<Entity, N> entities;
    const int total_flow;
    const unsigned int visited_valves;
    bool good = true;

    explicit State2(const SolverInfo &info)
        : entities(create_array<N>(Entity(info.initial_pos, 0))), total_flow(0),
          visited_valves(0) {}

    State2(int total_flow, unsigned int visited_valves,
           const std::array<Entity, N> &new_entities)
        : entities(new_entities), total_flow(total_flow),
          visited_valves(visited_valves) {}

    State2(int total_flow, unsigned int visited_valves,
           std::convertible_to<Entity> auto &&...new_entities)
        : entities{new_entities...}, total_flow(total_flow),
          visited_valves(visited_valves) {}

    template <std::size_t... Is>
    State2 move_some(int future_value,
                     std::convertible_to<const Entity> auto &&...Es) const {
        std::array<Entity, N> new_entities;
        // std::transform(entities.cbegin(), entities.cend(),
        // new_entities.begin(),
        //                [](const Entity &entity) { return entity.travel(); });
        unsigned int new_valves = visited_valves;
        std::initializer_list<std::size_t> indices{Is...};
        std::initializer_list<const Entity> moved_entities{Es...};
        auto i_it = indices.begin();
        auto moved_it = moved_entities.begin();
        for (std::size_t i = 0; i < N; ++i) {
            if (i_it != indices.end() && i == *i_it) {
                new_entities[i] = *moved_it;
                new_valves |= 1 << moved_it->pos;
                ++i_it;
                ++moved_it;
            } else {
                new_entities[i] = entities[i].travel();
            }
        }
        return State2(total_flow + future_value, new_valves, new_entities);
    }

    int flow_upper_bound(const SolverInfo &info, int remaining_time) const {
        int max_flow = total_flow;
        for (Key key = 0; key < info.graph.valves.size(); ++key) {
            if (visited_valves & (1 << key)) {
                continue;
            }
            int min_distance = std::numeric_limits<int>::max();
            for (const auto &entity : entities) {
                min_distance =
                    std::min(min_distance,
                             info.dists[entity.pos][key] + entity.travel_time);
            }
            int future_value = (remaining_time - (min_distance + 1)) *
                               info.graph.valves[key]->flow_rate;
            if (future_value <= 0) {
                continue;
            }
            max_flow += future_value;
        }
        return max_flow;
    }
};

template <bool use_entity_2>
int solve_bfs_2(const SolverInfo &info, int total_time) {
    using state_t = std::conditional_t<use_entity_2, State2<2>, State2<1>>;
    std::vector<state_t> curr_queue{{state_t(info)}};
    std::vector<state_t> next_queue{};

    int best_total = 0;
    for (int remaining_time = total_time; remaining_time > 0;
         --remaining_time) {
        int state_count = 0;
        int best_actual_flow = 0;
        for (auto it = curr_queue.begin(); it != curr_queue.end(); ++it) {
            const state_t &state = *it;
            if (!state.good) {
                continue;
            }
            if (state.total_flow > best_total) {
                best_total = state.total_flow;
            }
            if (std::ranges::all_of(state.entities, [](const Entity &entity) {
                    return entity.travel_time > 0;
                })) {
                next_queue.push_back(state.template move_some<>(0));
                if (next_queue.back().total_flow > best_actual_flow) {
                    best_actual_flow = next_queue.back().total_flow;
                }
                continue;
            }
            ++state_count;
            const auto &distances_1 = info.dists[state.entities.at(0).pos];
            // const auto &distances_2 = info.dists[state.entities.at(1).pos];
            for (Key key_a = 0; key_a < info.graph.valves.size(); ++key_a) {
                int flow_rate = info.graph.valves[key_a]->flow_rate;
                if (state.visited_valves & (1 << key_a) || flow_rate == 0) {
                    // skip valves we've already opened
                    continue;
                }
                // calculate total future value for an unopened valve, given
                // the distance to the valve and the remaining time
                int distance_a = -1;
                if (!use_entity_2 || state.entities[0].travel_time == 0) {
                    distance_a = distances_1[key_a];
                } else {
                    distance_a = info.dists[state.entities.at(1).pos][key_a];
                }
                // deduct the travel time plus the minute it takes to open
                // the valve
                int future_value_a =
                    (remaining_time - (distance_a + 1)) * flow_rate;
                if (future_value_a <= 0) {
                    continue;
                }
                if (!use_entity_2 || state.entities[1].travel_time > 0) {
                    // move entity 1
                    next_queue.push_back(state.template move_some<0>(
                        future_value_a, Entity(key_a, distance_a)));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                    continue;
                }
                if (state.entities[0].travel_time > 0) {
                    // move entity 2
                    next_queue.push_back(state.template move_some<1>(
                        future_value_a, Entity(key_a, distance_a)));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                    continue;
                }
                // loop over possible destinations for entity 2
                assert(state.entities[0].travel_time == 0 &&
                       state.entities[1].travel_time == 0);
                for (Key key_b = 0; key_b < info.graph.valves.size(); ++key_b) {
                    if (key_b == key_a) {
                        continue;
                    }
                    flow_rate = info.graph.valves[key_b]->flow_rate;
                    if (state.visited_valves & (1 << key_b) || flow_rate == 0) {
                        // skip valves we've already opened
                        continue;
                    }
                    // calculate total future value for an unopened valve,
                    // given the distance to the valve and the remaining
                    // time
                    int distance_b =
                        info.dists[state.entities.at(1).pos][key_b];
                    // deduct the travel time plus the minute it takes to
                    // open the valve
                    int future_value_b =
                        (remaining_time - (distance_b + 1)) * flow_rate;
                    if (future_value_b <= 0) {
                        continue;
                    }
                    next_queue.push_back(state.template move_some<0, 1>(
                        future_value_a + future_value_b,
                        Entity(key_a, distance_a), Entity(key_b, distance_b)));
                    if (next_queue.back().total_flow > best_actual_flow) {
                        best_actual_flow = next_queue.back().total_flow;
                    }
                }
            }
        }
        int reject_count = 0;
        for (auto it = next_queue.begin(); it != next_queue.end(); ++it) {
            if (it->flow_upper_bound(info, remaining_time - 1) <
                best_actual_flow) {
                it->good = false;
                ++reject_count;
            }
        }
        if constexpr (aoc::DEBUG) {
            std::cerr << "remaining time = " << total_time - remaining_time + 1
                      << ": " << state_count << " states, "
                      << next_queue.size() - reject_count << " branches ("
                      << reject_count << " rejected)\n";
        }
        curr_queue.clear();
        std::swap(curr_queue, next_queue);
    }
    return best_total;
}

template <std::size_t N, std::size_t M>
void produce_states(const SolverInfo &, std::vector<State2<N>> &next_queue,
                    const int &, int new_flow, unsigned int new_visited,
                    const std::array<Entity, M> new_entities) {
    static_assert(M == N);
    next_queue.emplace_back(new_flow, new_visited, new_entities);
}

template <std::size_t N, std::size_t M>
void produce_states(const SolverInfo &info, std::vector<State2<N>> &next_queue,
                    const int &remaining_time, int new_flow,
                    unsigned int new_visited,
                    const std::array<Entity, M> &prev_entities,
                    const Entity &curr_entity,
                    std::convertible_to<const Entity &> auto &&...rest) {
    static_assert(M + 1 + sizeof...(rest) == N);
    if (curr_entity.travel_time > 0) {
        std::array<Entity, M + 1> new_entities;
        std::ranges::copy(prev_entities, new_entities.begin());
        std::get<M>(new_entities) = curr_entity.travel();
        produce_states<N, M + 1>(info, next_queue, remaining_time, new_flow,
                                 new_visited, new_entities, rest...);
    } else {
        const auto &distances = info.dists[curr_entity.pos];
        for (Key key = 0, mask = 1; key < info.graph.valves.size();
             ++key, mask <<= 1) {

            int flow_rate = info.graph.valves[key]->flow_rate;
            if (new_visited & mask || flow_rate == 0) {
                // skip valves we've already opened
                continue;
            }
            // calculate total future value for an unopened valve, given
            // the distance to the valve and the remaining time
            int distance = distances[key];
            // deduct the travel time plus the minute it takes to open
            // the valve
            int future_value = (remaining_time - (distance + 1)) * flow_rate;
            if (future_value <= 0) {
                continue;
            }

            // move current entity
            std::array<Entity, M + 1> new_entities;
            std::ranges::copy(prev_entities, new_entities.begin());
            std::get<M>(new_entities) = Entity(key, distance);
            produce_states<N, M + 1>(info, next_queue, remaining_time,
                                     new_flow + future_value,
                                     new_visited | mask, new_entities, rest...);
        }
    }
}

template <std::size_t N>
void produce_states(const SolverInfo &info, std::vector<State2<N>> &next_queue,
                    const int &remaining_time, const State2<N> &state) {
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        produce_states<N>(info, next_queue, remaining_time, state.total_flow,
                          state.visited_valves, std::array<Entity, 0>{},
                          std::get<I>(state.entities)...);
    }
    (std::make_index_sequence<N>{});
}

template <int N>
int solve_bfs_3(const SolverInfo &info, int total_time) {
    using state_t = State2<N>;
    std::vector<state_t> curr_queue{{state_t(info)}};
    std::vector<state_t> next_queue{};

    int best_total = 0;
    for (int remaining_time = total_time; remaining_time > 0;
         --remaining_time) {
        int best_actual_flow = 0;
        std::size_t next_i = 0;
        for (auto it = curr_queue.begin(); it != curr_queue.end(); ++it) {
            const state_t &state = *it;
            if (!state.good) {
                continue;
            }
            if (state.total_flow > best_total) {
                best_total = state.total_flow;
            }
            produce_states(info, next_queue, remaining_time, state);
            for (; next_i < next_queue.size(); ++next_i) {
                if (next_queue[next_i].total_flow > best_actual_flow) {
                    best_actual_flow = next_queue[next_i].total_flow;
                }
            }
        }
        int reject_count = 0;
        for (auto it = next_queue.begin(); it != next_queue.end(); ++it) {
            if (it->flow_upper_bound(info, remaining_time - 1) <
                best_actual_flow) {
                it->good = false;
                ++reject_count;
            }
        }
        if constexpr (aoc::DEBUG) {
            std::cerr << "remaining time = " << total_time - remaining_time + 1
                      << ": " << next_queue.size() - reject_count
                      << " branches (" << reject_count << " rejected)\n";
        }
        curr_queue.clear();
        std::swap(curr_queue, next_queue);
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
    SolverInfo info{graph, dists};
    int total = 0;
    constexpr int N = 1;
    for (int i = 0; i < N; ++i) {
        // DFSSolver solver_1{info, 30};
        // total += solver_1.solve();
        // total += solve_bfs(info, 30);
        total += solve_bfs_3<1>(info, 30);
    }
    std::cout << total / N << "\n";

    // DFSSolver solver_2{info, 26, 26};
    // std::cout << solver_2.solve() << "\n";
    // std::cout << solve_bfs(info, 26, true) << "\n";
    std::cout << solve_bfs_3<2>(info, 26) << "\n";

    return 0;
}
