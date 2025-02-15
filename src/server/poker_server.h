#pragma once

#include <iostream>

#include "network_manager.h"
#include "table.h"

class PokerServer {
public:
    PokerServer();
    ~PokerServer();

    void run();
private:
    NetworkManager network;
};