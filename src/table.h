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
#include "game_packet.pb.h"
#include "network_manager.h"

static const uint16_t MAX_NUMBER_OF_COMMUNITY_CARDS = 5;

using PlayerList = std::vector<std::shared_ptr<Player>>;
using SocketToPlayerMap = std::unordered_map<int, std::shared_ptr<Player>>;
class Table {
private:
    PlayerList players_at_table;
    SocketToPlayerMap socket_to_player;
    std::vector<int> player_sockets;
    GameState current_game_state;
    int current_dealer;
    std::unique_ptr<Deck> deck;

    PlayerList build_players(const std::vector<int>& players, std::mt19937& generator);
    uint64_t generate_token(std::unordered_set<uint64_t>& unique_tokens
            , std::uniform_int_distribution<uint64_t>& dist
            , std::mt19937& generator) const;

    // game function helpers

    // dealing community cards 
    void deal_hands(int& deck_idx);
    void deal_community_card(int& deck_idx, std::vector<Card>& community_cards);
    void deal_flop(int& deck_idx, std::vector<Card>& community_cards);
    void deal_turn_or_river(int& deck_idx, std::vector<Card>& community_cards);
    
    // helpers for each key point in a hand
    int handle_betting_action(bool is_pre_flop, std::unordered_set<int>& excluded_players, int& current_player_action, int& pot_size, int& deck_idx);
    std::vector<int> decide_winners(const std::vector<int>& remaining_players, const std::vector<Card>& community_cards);
    void award_chips_to_winner(int winner, int amount);
    void award_chips_to_winners(const std::vector<int>& winners, int amount);

    // deciding winner helpers
    HandTieBreakInfo determine_hand_strength(int player_idx, const std::vector<Card>& community_cards);
    std::vector<int> build_five_card_hand(int player_idx, const std::vector<Card>& community_cards);
    
    // reset
    // print_decision

    // other helpers
    void update_player_idx(int& player_idx, std::unordered_set<int>& excluded_players);
    void take_blinds(int& player_idx, std::unordered_set<int>& excluded_players);
    std::vector<int> get_remaining_players(const std::unordered_set<int> excluded_players);

public:
    Table(const std::vector<int>& players);

    // game functions
    void shuffle_deck();
    void update_dealer();
    void play_hand();

    void broadcast_to_players(const GamePacket& game_packet) const;
};