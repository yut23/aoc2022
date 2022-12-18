/******************************************************************************
 * File:        day15.cpp
 *
 * Author:      yut23
 * Created:     2022-12-17
 *****************************************************************************/

#include "lib.h"
#include <cstdlib>  // for abs
#include <iostream> // for cout
#include <regex>    // for regex
#include <set>      // for set
#include <string>   // for string, getline, stoi
#include <vector>   // for vector

namespace aoc::day15 {

using Pos = aoc::Pos;

struct Sensor {
    Pos position;
    Pos nearest_beacon;

    Sensor(const Pos &position, const Pos &nearest_beacon)
        : position(position), nearest_beacon(nearest_beacon) {}

    int distance() const {
        return (nearest_beacon - position).manhattan_distance();
    }
};

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
                          << " (distance=" << sensors.back().distance()
                          << ")\n";
            }
        }
    }
    int target_row = 2000000;
    if (sensors.size() == 14 && sensors.front().position == Pos(2, 18)) {
        // example uses a different value
        target_row = 10;
    }
    std::set<Pos> covered{};
    for (const auto &sensor : sensors) {
        int max_i =
            sensor.distance() - std::abs(target_row - sensor.position.y);
        Pos p{sensor.position.x - max_i, target_row};
        for (int i = -max_i; i <= max_i; ++i, ++p.x) {
            if (!beacons.contains(p)) {
                covered.insert(p);
            }
        }
    }
    std::cout << covered.size() << std::endl;
    return 0;
}
