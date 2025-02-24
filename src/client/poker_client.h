#pragma once

#include <iostream>

#include "network_manager.h"

class PokerClient {
public:
    PokerClient();
    ~PokerClient();

    void run(const char* ip_address);
    void print_board(const GamePacket& game_packet) const;
    void print_card(const Card& card) const;
private:
    NetworkManager network;
};