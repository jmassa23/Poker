#pragma once

#include <iostream>

#include "network_manager.h"

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
    void print_card(const Card& card) const;

};