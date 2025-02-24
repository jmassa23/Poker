#pragma once

#include <iostream>

#include "network_manager.h"
#include "table.h"

using PokerTable = std::unique_ptr<Table>;
class PokerServer {
public:
    PokerServer();
    ~PokerServer();

    void run();
    void play_game() const;
    void play_hand() const;
    void send_game_start_message(GamePacket& game_packet) const;
    void print_game_info(const PokerTable& table) const;

private:
    NetworkManager network;
};