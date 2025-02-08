#pragma once

#include <iostream>

#include "../network_manager.h"

class PokerClient {
public:
    PokerClient();
    ~PokerClient();

    void run(const char* ip_address);
private:
    NetworkManager network;
};