/******************************************************************************
 * File:        day15.cpp
 *
 * Author:      yut23
 * Created:     2022-12-17
 *****************************************************************************/

#include "lib.h"
#include <algorithm> // for count_if
#include <cassert>   // for assert
#include <cstdlib>   // for abs
#include <iostream>  // for cout, cerr
#include <iterator>  // for next
#include <map>       // for map
#include <memory>    // for shared_ptr, make_shared
#include <regex>     // for regex, regex_match, smatch
#include <set>       // for set
#include <stdexcept> // for domain_error
#include <string>    // for string, getline, stoi
#include <vector>    // for vector

namespace aoc::day15 {

using Pos = aoc::Pos;

struct Sensor {
    const Pos position;
    const Pos nearest_beacon;
    const int distance;

    Sensor(const Pos &position, const Pos &nearest_beacon)
        : position(position), nearest_beacon(nearest_beacon),
          distance((nearest_beacon - position).manhattan_distance()) {}
};

// represents the range from start to end-1
struct RangeElement {
    const int start;
    int end;

    RangeElement(int start, int end) : start(start), end(end) {
        if (end <= start) {
            throw std::domain_error("end must be strictly greater than start");
        }
    }

    int length() const { return end - start; }

    bool is_start(int index) const { return index == start; }
    bool is_end(int index) const { return index == end; }
};

std::ostream &operator<<(std::ostream &os, const RangeElement &elt) {
    os << '[' << elt.start << ", " << elt.end << ')';
    return os;
}

// stores a set of covered ranges; supports adding new ranges and checking point
// membership
class RangeLookup {
    using pointer = std::shared_ptr<RangeElement>;
    using iterator = std::map<int, pointer>::iterator;
    using const_iterator = std::map<int, pointer>::const_iterator;

    std::map<int, pointer> table{};

    // moves old_end to new_end, removes any subsumed ranges in between, and
    // merges with the range starting at new_end if present
    void expand_range(const_iterator start, int new_end);

    // constructs a new RangeElement and inserts it into the table
    iterator create_new_range(int start);

    // internal validity check
    void check_invariants() const;

  public:
    int count_covered() const;
    void add_range(int start, int end);

    friend std::ostream &operator<<(std::ostream &, const RangeLookup &);
};

std::ostream &operator<<(std::ostream &os, const RangeLookup &lookup) {
    bool first = true;
    for (const auto &[index, elt] : lookup.table) {
        if (elt->is_start(index)) {
            if (!first) {
                os << "; ";
            }
            os << "[" << index << ", ";
        } else {
            assert(elt->is_end(index));
            os << index - 1 << "]";
        }
        first = false;
    }
    return os;
}

void RangeLookup::add_range(int start, int end) {
    if constexpr (aoc::DEBUG) {
        std::cerr << "adding range [" << start << ", " << end << ")\n";
    }
    check_invariants();
    const_iterator lower_bound = table.lower_bound(start);
    if (lower_bound == table.end()) {
        if constexpr (aoc::DEBUG) {
            std::cerr << "at the end of the table, creating a new range\n";
        }
        expand_range(create_new_range(start), end);
    } else {
        int lower_pos = lower_bound->first;
        pointer lower_range = lower_bound->second;
        if (lower_range->is_start(lower_pos) && lower_pos != start) {
            if constexpr (aoc::DEBUG) {
                std::cerr << "outside existing ranges, creating a new range\n";
            }
            expand_range(create_new_range(start), end);
        } else {
            // update the end point of the existing range
            if constexpr (aoc::DEBUG) {
                std::cerr << "inside existing range, merging\n";
            }
            const_iterator start_it = table.find(lower_range->start);
            expand_range(start_it, end);
        }
    }
    check_invariants();
}

RangeLookup::iterator RangeLookup::create_new_range(int start) {
    pointer elt = std::make_shared<RangeElement>(start, start + 1);
    return table.emplace(start, elt).first;
}

void RangeLookup::expand_range(const_iterator start_it, int new_end) {
    const pointer elt = start_it->second;
    if (new_end < elt->end) {
        if constexpr (aoc::DEBUG) {
            std::cerr
                << "new range is entirely within existing one, returning\n";
        }
        return;
    }
    const_iterator end_it = table.find(elt->end);
    if (end_it != table.end() && end_it->second == elt) {
        // remove old end pointer for this range
        if constexpr (aoc::DEBUG) {
            std::cerr << "removing old end pointer at " << elt->end << "\n";
        }
        table.erase(end_it);
    }
    elt->end = new_end;
    // remove any ranges that the new range will overlap with
    while (true) {
        const_iterator it = std::next(start_it);
        if (it == table.end() || it->first > new_end) {
            break;
        }
        assert(it->second->is_start(it->first));
        // range to subsume or merge
        if (it->second->end > new_end) {
            // update new_end in order to merge with the overlapping range
            new_end = it->second->end;
            if constexpr (aoc::DEBUG) {
                std::cerr << "merging range " << *(it->second) << " into "
                          << *elt << " (new_end=" << new_end << ")\n";
            }
        } else if constexpr (aoc::DEBUG) {
            std::cerr << "subsuming range " << *(it->second) << "\n";
        }
        // remove subsumed/merged range
        table.erase(it, std::next(it, 2));
    }
    // update end value
    elt->end = new_end;
    // insert end pointer at new_end, and make sure the insert was successful
    assert(table.emplace(new_end, elt).second);
}

void RangeLookup::check_invariants() const {
    assert(table.size() % 2 == 0);
    if constexpr (aoc::DEBUG) {
        std::cerr << "checking invariants...\n";
        for (const_iterator it = table.cbegin(); it != table.cend(); ++it) {
            std::cerr << "  checking " << *(it->second) << " at " << it->first
                      << "\n";
            const auto &[index, elt] = *it;
            int other_index;
            const_iterator other_it = it;
            if (index == elt->start) {
                ++other_it;
                other_index = elt->end;
            } else {
                --other_it;
                other_index = elt->start;
                // make sure different ranges aren't adjacent
                const_iterator next_it = std::next(it);
                if (next_it != table.end()) {
                    assert(next_it->first > index);
                }
            }
            assert(other_it != table.end());
            std::cerr << "  other_index: " << other_index << "\n";
            std::cerr << "  other_it: " << other_it->first << " -> "
                      << *(other_it->second) << "\n";
            assert(other_it->first == other_index);
            assert(other_it->second == elt);
            assert(table.find(other_index) == other_it);
        }
        std::cerr << "\n";
    }
}

int RangeLookup::count_covered() const {
    int count = 0;
    for (const auto &[index, elt] : table) {
        if (elt->is_start(index)) {
            count += elt->length();
        }
    }
    return count;
}

bool part_2_helper(const std::vector<Sensor> &sensors, const Pos &p,
                   int max_coord) {
    if (p.x < 0 || p.x > max_coord || p.y < 0 || p.y > max_coord) {
        return false;
    }
    for (const Sensor &sensor : sensors) {
        if ((p - sensor.position).manhattan_distance() <= sensor.distance) {
            return false;
        }
    }
    return true;
}

Pos solve_part_2(const std::vector<Sensor> &sensors, int max_coord) {
    // for each sensor, check each point just outside the perimeter
    for (const Sensor &sensor : sensors) {
        Pos north{sensor.position}, east{sensor.position},
            south{sensor.position}, west{sensor.position};
        north.y -= sensor.distance + 1;
        east.x += sensor.distance + 1;
        south.y += sensor.distance + 1;
        west.x -= sensor.distance + 1;

        constexpr Delta south_east{+1, +1};
        constexpr Delta south_west{-1, +1};
        constexpr Delta north_west{-1, -1};
        constexpr Delta north_east{+1, -1};

        for (int i = 0; i <= sensor.distance; ++i) {
            if (part_2_helper(sensors, north, max_coord)) {
                return north;
            }
            if (part_2_helper(sensors, east, max_coord)) {
                return east;
            }
            if (part_2_helper(sensors, south, max_coord)) {
                return south;
            }
            if (part_2_helper(sensors, west, max_coord)) {
                return west;
            }
            // move south-east
            north += south_east;
            // move south-west
            east += south_west;
            // move north-west
            south += north_west;
            // move north-east
            west += north_east;
        }
    }
    assert(false);
}

} // namespace aoc::day15

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day15;
    std::vector<Sensor> sensors{};
    std::set<Pos> beacons{};
    // read file line-by-line
    std::string line;
    std::regex line_regex{
        R"(Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+))"};
    while (std::getline(infile, line)) {
        std::smatch line_match;
        if (std::regex_match(line, line_match, line_regex)) {
            Pos sensor_pos{std::stoi(line_match[1]), std::stoi(line_match[2])};
            Pos beacon_pos{std::stoi(line_match[3]), std::stoi(line_match[4])};
            sensors.emplace_back(sensor_pos, beacon_pos);
            beacons.insert(beacon_pos);
            if constexpr (aoc::DEBUG) {
                std::cerr << "sensor at " << sensors.back().position
                          << ", nearest beacon at "
                          << sensors.back().nearest_beacon
                          << " (distance=" << sensors.back().distance << ")\n";
            }
        }
    }
    int target_row = 2000000;
    if (sensors.size() == 14 && sensors.front().position == Pos(2, 18)) {
        // example uses a different value
        target_row = 10;
    }
    int num_beacons = std::ranges::count_if(
        beacons, [=](const Pos &p) { return p.y == target_row; });

    // use a binary search tree to store the start and end positions of each
    // sensor's range
    RangeLookup lookup{};
    for (const Sensor &sensor : sensors) {
        int half_width =
            sensor.distance - std::abs(target_row - sensor.position.y);
        if (half_width >= 0) {
            lookup.add_range(sensor.position.x - half_width,
                             sensor.position.x + half_width + 1);
            if constexpr (aoc::DEBUG) {
                std::cerr << "ranges: " << lookup << "\n";
            }
        }
    }
    std::cout << lookup.count_covered() - num_beacons << std::endl;

    /* Part 2 strategy:
     * Given that there's only one possible position, it must be just outside
     * the perimeter of a sensor's range (otherwise there would be more than
     * one). Just checking those positions reduces the search space from O(N^2)
     * to O(N).
     */

    Pos beacon_pos = solve_part_2(sensors, target_row * 2);
    std::cout << beacon_pos.x * 4000000L + beacon_pos.y << std::endl;

    return 0;
}
