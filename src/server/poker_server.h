#pragma once

#include <iostream>

#include "../network_manager.h"

class PokerServer {
public:
    PokerServer();
    ~PokerServer();

    void run();
private:
    NetworkManager network;
};