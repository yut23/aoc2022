/******************************************************************************
 * File:        day24.cpp
 *
 * Author:      yut23
 * Created:     2022-12-26
 *****************************************************************************/

#include "lib.h"
#include <iomanip>  // for quoted
#include <iostream> // for cout
#include <memory>   // for shared_ptr, make_shared, enable_shared_from_this
#include <queue>    // for queue
#include <set>      // for set
#include <string>   // for string, getline
#include <utility>  // for pair
#include <vector>   // for vector

namespace aoc::day24 {
using Pos = aoc::Pos;
using Direction = aoc::Direction;

struct Blizzard {
    Pos pos;
    Direction direction;

    Blizzard(const Pos &pos, const Direction &direction)
        : pos(pos), direction(direction) {}

    void move(int width, int height) {
        switch (direction) {
        case Direction::up:
            // add (height - 1) to avoid negative numbers
            pos.y = (pos.y + height - 1) % height;
            break;
        case Direction::down:
            pos.y = (pos.y + 1) % height;
            break;
        case Direction::left:
            // add (width - 1) to avoid negative numbers
            pos.x = (pos.x + width - 1) % width;
            break;
        case Direction::right:
            pos.x = (pos.x + 1) % width;
            break;
        }
    }
};

class Valley {
  public:
    const int width;
    const int height;
    const Pos entrance, exit;

  private:
    std::vector<Blizzard> blizzards{};

    int time;
    // the positions of the blizzards at the next time step
    std::vector<std::vector<std::uint8_t>> blizzard_counts;

    void advance_time();

    bool in_bounds(const Pos &pos) const {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

  public:
    explicit Valley(const std::vector<std::string> &lines);
    int bfs(const Pos &src, const Pos &dest);
};

Valley::Valley(const std::vector<std::string> &lines)
    : width(lines[0].size() - 2), height(lines.size() - 2), entrance(0, -1),
      exit(width - 1, height) {
    // subtract 2 from width and height for the walls

    blizzard_counts = std::vector<std::vector<std::uint8_t>>(
        width, std::vector<std::uint8_t>(height, 0));

    time = 0;
    Pos pos{0, 0};
    for (unsigned int i = 1; i < lines.size() - 1; ++i) {
        for (char c : lines[i].substr(1, width)) {
            if (c != '.') {
                Direction direction;
                switch (c) {
                case '^':
                    direction = Direction::up;
                    break;
                case 'v':
                    direction = Direction::down;
                    break;
                case '<':
                    direction = Direction::left;
                    break;
                case '>':
                    direction = Direction::right;
                    break;
                default:
                    std::cerr << "got invalid character in input: "
                              << std::quoted(std::string(1, c)) << "\n";
                    assert(false);
                    break;
                }
                blizzards.emplace_back(pos, direction);
                blizzards.back().move(width, height);
                ++blizzard_counts[blizzards.back().pos.x]
                                 [blizzards.back().pos.y];
            }
            ++pos.x;
        }
        pos.x = 0;
        ++pos.y;
    }
}

void Valley::advance_time() {
    // update blizzard positions and counts
    for (Blizzard &blizzard : blizzards) {
        --blizzard_counts[blizzard.pos.x][blizzard.pos.y];
        blizzard.move(width, height);
        ++blizzard_counts[blizzard.pos.x][blizzard.pos.y];
    }
    if constexpr (aoc::DEBUG) {
        // make sure there aren't too many blizzards in any one spot
        for (const auto &row : blizzard_counts) {
            for (const auto count : row) {
                assert(count <= 4);
            }
        }
    }
    ++time;
}

int Valley::bfs(const Pos &src, const Pos &dest) {
    std::set<Pos> curr_positions{{src}};
    std::set<Pos> next_positions{};

    while (true) {
        for (const auto &pos : curr_positions) {
            for (const Direction &dir : {Direction::up, Direction::down,
                                         Direction::left, Direction::right}) {
                Delta delta{dir};
                delta.dy *= -1;
                Pos candidate = pos + delta;
                if (candidate == dest) {
                    advance_time();
                    return time;
                }
                if (!in_bounds(candidate)) {
                    // out-of-bounds
                    continue;
                }
                if (blizzard_counts[candidate.x][candidate.y] > 0) {
                    // would be blocked by a blizzard
                    continue;
                }
                next_positions.emplace(candidate);
            }
            if (pos == src ||
                (in_bounds(pos) && blizzard_counts[pos.x][pos.y] == 0)) {
                next_positions.emplace(pos);
            }
        }
        curr_positions.clear();
        // advance time and swap the queues
        advance_time();
        assert(!next_positions.empty());
        std::swap(curr_positions, next_positions);
    }
}

} // namespace aoc::day24

int main(int argc, char **argv) {
    std::ifstream infile = aoc::parse_args(argc, argv);

    using namespace aoc::day24;
    // read file line-by-line
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    Valley valley{lines};

    // part 1
    std::cout << valley.bfs(valley.entrance, valley.exit) << "\n";
    // go back for the snacks
    valley.bfs(valley.exit, valley.entrance);
    // return to the exit again
    std::cout << valley.bfs(valley.entrance, valley.exit) << "\n";
    return 0;
}
