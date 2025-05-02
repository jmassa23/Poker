#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <ranges>

#include "utils.h"
#include "game_packet.pb.h"

static const uint16_t DECK_SIZE = 52;

class Deck {
private:
    std::vector<Card> deck;
    std::mt19937 gen;
public:
    Deck();
    void shuffle();

    Card get_card(int index);
};