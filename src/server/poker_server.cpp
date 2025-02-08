#include "poker_server.h"

PokerServer::PokerServer() : network() {}

PokerServer::~PokerServer() {}

void PokerServer::run() {
    bool networkRunning = network.initialize_server();
    if(!networkRunning){
        perror("Error: Could not initialize server.");
        return;
    }

    int num_players=0;
    while(num_players < 1){
        int new_client;
        if((new_client=network.accept_client()) == -1){
            perror("Error: Could not accept client.");
            continue;
        }
        std::cout << "client added" << std::endl;
        num_players++;
    }
}