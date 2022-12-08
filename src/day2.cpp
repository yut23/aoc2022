/******************************************************************************
 * File:        day2.cpp
 *
 * Author:      yut23
 * Created:     2022-12-02
 *****************************************************************************/

#include "lib.h"
#include <array>
#include <cassert>
#include <string>

int main(int argc, char **argv) {
    auto infile = aoc::parse_args(argc, argv);

    // read file line-by-line
    char opponent_letter, own_letter;
    std::array<int, 2> score{0, 0};
    int round = 0;
    while (infile >> opponent_letter >> own_letter) {
        ++round;
        // shift letter to be equal to the point value
        int opponent_hand = opponent_letter - 'A' + 1;
        for (int part = 1; part <= 2; ++part) {
            int own_hand = own_letter - 'X' + 1;
            if (part == 2) {
                // yuck.
                own_hand = ((opponent_hand + (own_hand + 1) % 3) - 1) % 3 + 1;
            }
            if constexpr (aoc::DEBUG) {
                std::cerr << "round " << round << ", part " << part << ":";
                std::cerr << " hand=" << own_hand << ",";
            }
            int round_score;
            switch ((opponent_hand - own_hand + 3) % 3) {
            case 0:
                // draw
                round_score = 3;
                break;
            case 1:
                // opponent wins
                round_score = 0;
                break;
            case 2:
                // you win
                round_score = 6;
                break;
            default:
                assert(0);
                break;
            }
            if constexpr (aoc::DEBUG) {
                std::cerr << " score=" << round_score << std::endl;
            }
            score[part - 1] += round_score + own_hand;
        }
    }
    std::cout << score[0] << std::endl;
    std::cout << score[1] << std::endl;
    return EXIT_SUCCESS;
}
