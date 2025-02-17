#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <ranges>

#include "utils.h"
#include "game_packet.pb.h"

class Deck {
private:
    std::vector<Card> deck;
    std::mt19937 gen;
public:
    Deck();
    void shuffle();
};