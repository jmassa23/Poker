#pragma once

#include <string>
#include <utility>

#include "utils.h"

using string = std::string;
using Hand = std::pair<Card,Card>;

class Player {
private:
    string player_name;
    int player_id;
    Hand current_hand;
    int stack_size;
    int total_buy_in;
public:
    Player(string _player_name, int _player_id, int _buy_in);

    // getters 
    Hand get_hand();
    int get_stack_size();

    // modifiers by table on player
    void take_big_blind();
    void take_small_blind();
    void deal_card(const Card& card, bool has_card);
    void award_chips(int amount);
    void bet_chips(int amount);


};