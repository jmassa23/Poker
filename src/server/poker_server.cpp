#include "poker_server.h"

PokerServer::PokerServer() : network() {}

PokerServer::~PokerServer() {}

void PokerServer::run() {
    bool networkRunning = network.initialize_server();
    if(!networkRunning){
        perror("Error: Could not initialize server.");
        return;
    }
    std::cout << "Server Running." << std::endl;
    int max_players = 1;
    int num_players = 0;
    while(num_players < max_players){
        std::cout << "Waiting for " << max_players-num_players 
                  << " more players." << std::endl;

        int new_client;
        if((new_client = network.accept_client()) == -1){
            perror("Error: Could not accept client.");
            continue;
        }

        std::cout << "Client added" << std::endl;
        num_players++;
    }

    std::cout << "All players connected. Starting game." << std::endl;
}