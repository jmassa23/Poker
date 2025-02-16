#include "player.h"

Player::Player(string _player_name, int _player_id, int _buy_in) : 
            player_name(_player_name), player_id(_player_id), total_buy_in(_buy_in)
            , stack_size(_buy_in) {}