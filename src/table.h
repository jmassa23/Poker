#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <ranges>

#include "utils.h"
#include "deck.h"

class Table {
private:
    std::vector<int> players_at_table;
    std::unordered_map<int, int> sock_to_player_idx;
    GameState current_game_state;
    int current_dealer;
    std::unique_ptr<Deck> deck;
public:
    Table(const std::vector<int>& players);
};