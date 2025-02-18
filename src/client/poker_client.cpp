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

    GamePacket game_packet;
    NetworkManager::receive_from_server(socket, game_packet);
    if(game_packet.game_state() == GameState::SET_UP) {
        std::cout << "All players connected. Starting game." << std::endl;
    }
    else {
        std::cout << "Error: game state incorrect. Disconnecting from server." << std::endl;
        return;
    }


}