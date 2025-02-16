#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <ranges>

#include "utils.h"
#include "deck.h"
#include "player.h"

using PlayerList = std::vector<std::shared_ptr<Player>>;
using SocketToPlayerMap = std::unordered_map<int, std::shared_ptr<Player>>;
class Table {
private:
    PlayerList players_at_table;
    SocketToPlayerMap socket_to_player;
    GameState current_game_state;
    int current_dealer;
    std::unique_ptr<Deck> deck;

    PlayerList build_players(const std::vector<int>& players, std::mt19937& generator);
    uint64_t generate_token(std::unordered_set<uint64_t>& unique_tokens
            , std::uniform_int_distribution<uint64_t>& dist
            , std::mt19937& generator) const;
public:
    Table(const std::vector<int>& players);
};