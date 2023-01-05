/******************************************************************************
 * File:        day19.cpp
 *
 * Author:      yut23
 * Created:     2023-01-04
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for max, max_element
#include <array>     // for array
#include <cassert>   // for assert
#include <iostream>  // for cout, cerr
#include <limits>    // for numeric_limits
#include <vector>    // for vector

namespace aoc::day19 {

// not an enum class, since I want to be able to use the values as unscoped
// integer keys
enum ResourceType : unsigned char {
    ORE = 0,
    CLAY = 1,
    OBSIDIAN = 2,
    GEODE = 3,
};

template <typename T>
struct ResourceVector {
    std::array<T, 4> values;
    using value_type = T;
    using size_type = typename decltype(values)::size_type;

    constexpr T &operator[](size_type pos) { return values[pos]; }
    constexpr const T &operator[](size_type pos) const { return values[pos]; }

    template <typename U>
    ResourceVector &operator+=(const ResourceVector<U> &rhs) {
        values[ORE] += rhs[ORE];
        values[CLAY] += rhs[CLAY];
        values[OBSIDIAN] += rhs[OBSIDIAN];
        values[GEODE] += rhs[GEODE];
        return *this;
    }
    ResourceVector &operator+=(ResourceType rhs) {
        ++values[rhs];
        return *this;
    }
    template <typename U>
    ResourceVector &operator-=(const ResourceVector<U> &rhs) {
        values[ORE] -= rhs[ORE];
        values[CLAY] -= rhs[CLAY];
        values[OBSIDIAN] -= rhs[OBSIDIAN];
        values[GEODE] -= rhs[GEODE];
        return *this;
    }

    template <typename U>
    bool can_afford(const ResourceVector<U> &cost) const {
        return values[ORE] >= cost[ORE] && values[CLAY] >= cost[CLAY] &&
               values[OBSIDIAN] >= cost[OBSIDIAN];
    }

    bool operator==(const ResourceVector &) const = default;
};
template <typename T, typename U>
inline ResourceVector<T> operator+(ResourceVector<T> lhs,
                                   const ResourceVector<U> &rhs) {
    lhs += rhs;
    return lhs;
}
template <typename T>
inline ResourceVector<T> operator+(ResourceVector<T> lhs, ResourceType rhs) {
    lhs += rhs;
    return lhs;
}
template <typename T, typename U>
inline ResourceVector<T> operator-(ResourceVector<T> lhs,
                                   const ResourceVector<U> &rhs) {
    lhs -= rhs;
    return lhs;
}

using Resources = ResourceVector<short>;
using Robots = ResourceVector<char>;
using Cost = ResourceVector<char>;

struct Blueprint {
    int id = -1;
    std::array<Cost, 4> robot_costs{};
    Cost max_cost{};

    void update_max_costs() {
        // skip ore robot cost when calculating maximum
        max_cost[ORE] =
            std::max({robot_costs[CLAY][ORE], robot_costs[OBSIDIAN][ORE],
                      robot_costs[GEODE][ORE]});
        max_cost[CLAY] = robot_costs[OBSIDIAN][CLAY];
        max_cost[OBSIDIAN] = robot_costs[GEODE][OBSIDIAN];
        max_cost[GEODE] = std::numeric_limits<Cost::value_type>::max();
    }
};
std::istream &operator>>(std::istream &is, Blueprint &bp) {
    using namespace aoc;
    // "Blueprint <n>:"
    if (!(is >> skip(1))) {
        return is;
    }
    is >> bp.id >> skip(1);
    // "Each ore robot costs <n> ore."
    is >> skip(4) >> as_number{bp.robot_costs[ORE][ORE]} >> skip(1);
    // "Each clay robot costs <n> ore."
    is >> skip(4) >> as_number{bp.robot_costs[CLAY][ORE]} >> skip(1);
    // "Each obsidian robot costs <n> ore and <n> clay."
    is >> skip(4) >> as_number{bp.robot_costs[OBSIDIAN][ORE]} >> skip(2) >>
        as_number{bp.robot_costs[OBSIDIAN][CLAY]} >> skip(1);
    // "Each geode robot costs <n> ore and <n> obsidian."
    is >> skip(4) >> as_number{bp.robot_costs[GEODE][ORE]} >> skip(2) >>
        as_number{bp.robot_costs[GEODE][OBSIDIAN]} >> skip(1);
    bp.update_max_costs();
    return is;
}

// using a template parameter for remaining_time cuts the runtime in half
template <int remaining_time>
int find_best_dfs(const Blueprint &bp,
                  const Resources &resources = {0, 0, 0, 0},
                  const Robots &robots = {1, 0, 0, 0}) {
    if constexpr (remaining_time <= 0) {
        return resources[GEODE];
    } else {
        constexpr int new_time = remaining_time - 1;
        ResourceVector next_resources = resources;
        next_resources += robots;
        if (resources.can_afford(bp.robot_costs[GEODE])) {
            // always build a geode robot if we can afford it
            return find_best_dfs<new_time>(
                bp, next_resources - bp.robot_costs[GEODE],
                robots + ResourceType::GEODE);
        } else {
            int best = 0;
            for (ResourceType type : {OBSIDIAN, CLAY, ORE}) {
                if (robots[type] < bp.max_cost[type] &&
                    resources.can_afford(bp.robot_costs[type])) {
                    best = std::max(
                        best, find_best_dfs<new_time>(
                                  bp, next_resources - bp.robot_costs[type],
                                  robots + type));
                }
            }
            best = std::max(
                best, find_best_dfs<new_time>(bp, next_resources, robots));
            return best;
        }
    }
}

struct State {
    const Resources resources;
    const Robots robots;
    bool good = true;

    State() : resources({0, 0, 0, 0}), robots({1, 0, 0, 0}) {}
    State(const Resources &resources, const Robots &robots)
        : resources(resources), robots(robots) {}

    bool pareto_dominates(const State &other) const {
        // for one state to pareto dominate another, it must be no worse in any
        // category, and better in at least one.
        bool better = false;
        for (int i = 0; i < 4; ++i) {
            if (resources[i] < other.resources[i]) {
                return false;
            }
            if (resources[i] > other.resources[i]) {
                better = true;
            }
            if (robots[i] < other.robots[i]) {
                return false;
            }
            if (robots[i] > other.robots[i]) {
                better = true;
            }
        }
        return better;
    }

    bool operator==(const State &other) const {
        return resources == other.resources && robots == other.robots;
    }
};

int find_best_bfs(const Blueprint &bp, const int total_time) {
    if constexpr (aoc::DEBUG) {
        std::cerr << "\nBlueprint " << bp.id << ":\n";
    }
    std::vector<State> curr_queue{{State()}};
    std::vector<State> next_queue{};
    [[maybe_unused]] bool do_print = false;
    for (int remaining_time = total_time; remaining_time > 0;
         --remaining_time) {
        if constexpr (aoc::DEBUG) {
            if (curr_queue.size() > 1000) {
                do_print = true;
            }
            if (do_print) {
                std::cerr << "minute " << total_time - remaining_time + 1
                          << ":   " << curr_queue.size() << " branches\n";
            }
        }
        for (auto it = curr_queue.begin(); it != curr_queue.end(); ++it) {
            State &state = *it;
            if (!state.good) {
                continue;
            }
            // This is a trade-off between the O(n^2) cost of checking all pairs
            // for Pareto dominance and the O(c^t) growth of the search tree.
            // The search tree size seems to be more important at the start, so
            // this turns it off when we get close to the end.
            // ~5 works best for the full input, ~8 is best for the example
            if (remaining_time > 6) {
                for (auto other_it = it + 1; other_it != curr_queue.end();
                     ++other_it) {
                    if (*other_it == state ||
                        state.pareto_dominates(*other_it)) {
                        other_it->good = false;
                    } else if (other_it->pareto_dominates(state)) {
                        state.good = false;
                        break;
                    }
                }
            }
            if (!state.good) {
                continue;
            }
            const Resources next_resources = state.resources + state.robots;
            if (state.resources.can_afford(bp.robot_costs[GEODE])) {
                // always build a geode robot if we can afford it
                next_queue.emplace_back(
                    State(next_resources - bp.robot_costs[GEODE],
                          state.robots + GEODE));
            } else {
                for (ResourceType type : {OBSIDIAN, CLAY, ORE}) {
                    if (state.robots[type] < bp.max_cost[type] &&
                        state.resources.can_afford(bp.robot_costs[type])) {
                        next_queue.emplace_back(next_resources -
                                                    bp.robot_costs[type],
                                                state.robots + type);
                    }
                }
                next_queue.emplace_back(next_resources, state.robots);
            }
        }
        curr_queue.clear();
        std::swap(curr_queue, next_queue);
    }
    if constexpr (aoc::DEBUG) {
        if (do_print) {
            std::cerr << "final queue: " << curr_queue.size() << " branches\n";
        }
    }
    return std::ranges::max_element(
               curr_queue, {},
               [](const State &state) -> int { return state.resources[GEODE]; })
        ->resources[GEODE];
}

} // namespace aoc::day19

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day19;
    std::vector<Blueprint> first_three;
    int total_quality = 0;
    {
        Blueprint bp{};
        while (infile >> bp) {
            if (first_three.size() < 3) {
                first_three.push_back(bp);
            }
            int max_geodes = find_best_bfs(bp, 24);
            if constexpr (aoc::DEBUG) {
                std::cerr << "Blueprint " << bp.id
                          << ": max geodes opened = " << max_geodes << "\n";
            }
            total_quality += max_geodes * bp.id;
        }
    }

    std::cout << total_quality << "\n";
    const bool is_example = first_three.size() == 2;
    if (is_example) {
        assert(total_quality == 33);
    } else {
        assert(total_quality == 1092);
    }

    // part 2
    int product = 1;
    for (const Blueprint &bp : first_three) {
        int max_geodes = find_best_bfs(bp, 32);
        if constexpr (aoc::DEBUG) {
            std::cerr << "Blueprint " << bp.id
                      << ": max geodes opened = " << max_geodes << "\n";
        }
        if (is_example) {
            if (bp.id == 1) {
                assert(max_geodes == 56);
            } else if (bp.id == 2) {
                assert(max_geodes == 62);
            }
        } else {
            if (bp.id == 1) {
                assert(max_geodes == 14);
            } else if (bp.id == 2) {
                assert(max_geodes == 11);
            } else if (bp.id == 3) {
                assert(max_geodes == 23);
            }
        }
        product *= max_geodes;
    }

    std::cout << product << "\n";

    return 0;
}
