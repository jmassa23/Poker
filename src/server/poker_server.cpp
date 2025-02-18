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

    int max_players = 2;
    int num_players = 0;
    
    std::vector<int> players;
    players.reserve(max_players);
    while(num_players < max_players){
        std::cout << "Waiting for " << max_players-num_players 
                  << " more players." << std::endl;

        int new_client;
        if((new_client = network.accept_client()) == -1){
            perror("Error: Could not accept client.");
            continue;
        }

        std::cout << "Client added" << std::endl;
        players.push_back(new_client);
        num_players++;
    }

    network.set_up_client_sockets(players);
    std::cout << "All players connected. Starting game." << std::endl;

    // create a table and start the game.
    std::unique_ptr<Table> table = std::make_unique<Table>(network.get_client_sockets());
}