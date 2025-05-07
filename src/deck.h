#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <ranges>

#include "utils.h"
#include "game_packet.pb.h"

static const uint16_t DECK_SIZE = 52;
static const uint16_t ACE_LOW_RANK = 1;
static const uint16_t JACK_RANK = 11;
static const uint16_t QUEEN_RANK = 12;
static const uint16_t KING_RANK = 13;
static const uint16_t ACE_HIGH_RANK = 14;
class Deck {
private:
    std::vector<Card> deck;
    std::mt19937 gen;
public:
    Deck();
    void shuffle();

    Card get_card(int index);
};