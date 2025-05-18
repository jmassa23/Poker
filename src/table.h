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
    std::pair<HandRank, std::vector<int>> decide_winners(const std::vector<int>& remaining_players, const std::vector<Card>& community_cards);
    void award_chips_to_winner(int winner, int amount);
    void award_chips_to_winners(const std::vector<int>& winners, int amount);

    // deciding winner helpers
    std::vector<int> break_hand_rank_tie(std::vector<int> eligible_to_win, std::unordered_map<int, HandTieBreakInfo> hand_strengths);
    bool found_equal_or_higher_value_hand(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_card(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_quads(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_full_house(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_trips(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_two_pair(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    bool found_equal_or_higher_value_pair(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger);
    HandTieBreakInfo get_hand_strength(int player_idx, const std::vector<Card>& community_cards);
    std::vector<Card> build_combined_hand(int player_idx, const std::vector<Card>& community_cards);
    void determine_hand_strength(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info);
    void check_for_hand(HandRank& upper_bound_hand_rank, HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards);
    void check_for_straight_flush(HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards);
    void check_for_quads(HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards);
    void check_for_full_house(HandTieBreakInfo& hand_info);
    void check_for_straight_given_flush(HandTieBreakInfo& hand_info, const std::vector<Card>& flush_cards);
    void check_for_straight(HandTieBreakInfo& hand_info, const std::vector<Card>& cards);
    bool check_for_straight(const std::vector<Card>& cards, Card& high_card);
    void update_hand_rank_if_necessary(HandRank hand_rank, HandTieBreakInfo& hand_info);
    void get_additional_five_card_hand_data(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info);
    void fill_n_highest_cards(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info, int n);

    // sending updates to client helpers
    void send_player_stack_update(const std::unordered_set<int>& excluded_players);
    void send_player_action_update();
    void send_dealer_update(GameState game_state, const std::vector<Card>& community_cards);
    void send_hand_result(int winner, int pot_size);
    void send_hand_result(const std::vector<int> winners, int pot_size, HandRank hand_rank);
    // reset
    // print_decision

    // other helpers
    void initialize_excluded_players(std::unordered_set<int>& excluded_players);
    void update_player_idx(int& player_idx, std::unordered_set<int>& excluded_players);
    void take_blinds(int& player_idx, std::unordered_set<int>& excluded_players);
    std::vector<int> get_remaining_players(const std::unordered_set<int> excluded_players);

public:
    Table(const std::vector<int>& players);

    // game functions
    void shuffle_deck();
    void update_dealer();
    void play_hand();

    void send_game_start_message() const;
    void broadcast_to_players(const GamePacket& game_packet) const;
};