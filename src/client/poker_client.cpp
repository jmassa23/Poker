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

    retrieve_server_messages(socket);
}

void PokerClient::retrieve_server_messages(int socket) {
    while(true) {
        GamePacket game_packet;
        NetworkManager::receive_from_server(socket, game_packet);
        // if(game_packet.game_state() == GameState::SET_UP) {
        //     std::cout << "All players connected. Starting game." << std::endl;
        // }
        // else {
        //     std::cout << "Error: game state incorrect. Disconnecting from server." << std::endl;
        //     return;
        // }
    }
}

/*void PokerClient::print_board(const GamePacket& game_packet) const {
    std::ranges::for_each(game_packet.board(), [&](const Card& card){
        print_card(card);
    });
}*/

void PokerClient::print_card(const Card& card) const {
    switch(card.rank()) {
        case 14:
            std::cout << "A";
            break;
        case 13:
            std::cout << "K";
            break;
        case 12:
            std::cout << "Q";
            break;
        case 11:
            std::cout << "J";
            break;
        case 10:
            std::cout << "T";
            break;
        default:
            std::cout << card.rank();
            break;
    }

    switch(card.suit()) {
        case Suit::SPADE:
            std::cout << "\u2660";
            break;
        case Suit::HEART: // in red 
            std::cout << "\033[31m\u2665\033[0m";
            break;
        case Suit::CLUB:
            std::cout << "\u2663";
            break;
        case Suit::DIAMOND: // in red
            std::cout << "\033[31m\u2666\033[0m";
            break;
        default:
            std::cout << "ERROR: This text should never print." << std::endl;
            break;
    }

    std::cout << std::endl;
}