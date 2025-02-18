#include "poker_client.h"

PokerClient::PokerClient() : network() {}

PokerClient::~PokerClient() {}

void PokerClient::run(const char* ip_address) {
    int socket;
    if((socket = network.connect_to_server(ip_address)) == -1){
        perror("Error: Could not connect to server.");
        return;
    }

    std::cout << "You're connected to server. Waiting for all players to join." << std::endl;



}