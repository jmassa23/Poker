#include "player.h"

Player::Player(string _player_name, int _player_id, int _buy_in) : 
            player_name(_player_name), player_id(_player_id), total_buy_in(_buy_in)
            , stack_size(_buy_in) {}

Hand Player::get_hand() {
    return current_hand;
}

int Player::get_stack_size() {
    return stack_size;
}

void Player::take_big_blind() {
    stack_size -= 1;
}

// TODO - change stack_size to accomodate half values (aka a small blind)
void Player::take_small_blind() {
    stack_size -= 1;
}

void Player::deal_card(const Card& card, bool has_card) {
    if(has_card){
        current_hand.second = std::move(card);
    }
    else {
        current_hand.first = std::move(card);
    }
}

void Player::award_chips(int amount) {
    stack_size += amount;
}

void Player::bet_chips(int amount) {
    stack_size -= amount;
}