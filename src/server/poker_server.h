#pragma once

#include <iostream>
#include <csignal>

#include "network_manager.h"
#include "table.h"

using PokerTable = std::unique_ptr<Table>;
class PokerServer {
public:
    PokerServer();
    ~PokerServer();

    void run();
    void play_game(const std::vector<int>& client_sockets) const;
    void play_hand() const;
    void print_game_info(const PokerTable& table) const;
private:
    NetworkManager network;
};