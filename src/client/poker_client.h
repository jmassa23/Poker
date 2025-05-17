#pragma once

#include <iostream>

#include "network_manager.h"

using Hand = std::pair<Card,Card>;

class PokerClient {
public:
    PokerClient();
    ~PokerClient();

    void run(const char* ip_address);
    //void print_board(const GamePacket& game_packet) const;
    

    // helpers 
    //void run_game();

    // print functions 

    // info print functions
    //void start_game_message(GamePacket& game_packet);
    //void print_board(GamePacket& game_packet);

    // action print functions
private:
    NetworkManager network;

    void retrieve_server_messages(int socket);
    void print_board(const DealerUpdate& dealer_update) const;
    void print_card(const Card& card) const;
    void print_hand(const HoleCards& hand) const;

    void handle_player_stack_update(const PlayerStackUpdate& player_stack_update);
    void handle_player_action_update(const PlayerActionUpdate& player_action_update);
    void handle_dealer_update(const DealerUpdate& dealer_update);
    void handle_hand_result_update(const HandResult& hand_result);
};