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

        switch (game_packet.payload_case()) {
            case GamePacket::kStackUpdate:
                handle_player_stack_update(game_packet.stack_update());
                break;
            case GamePacket::kActionUpdate:
                handle_player_action_update(game_packet.action_update());
                break;
            case GamePacket::kDealerUpdate:
                handle_dealer_update(game_packet.dealer_update());
                break;
            case GamePacket::kHandResult:
                handle_hand_result_update(game_packet.hand_result());
                break;
            case GamePacket::PAYLOAD_NOT_SET:
                std::cerr << "Received GamePacket with no payload!" << std::endl;
                break;
        }
    }
}

void PokerClient::print_board(const DealerUpdate& dealer_update) const {
    std::ranges::for_each(dealer_update.community_cards(), [&](const Card& card){
        print_card(card);
    });
}

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

void PokerClient::handle_dealer_update(const DealerUpdate& dealer_update) {
    switch (dealer_update.game_state()) {
        case GameState::SET_UP:
            std::cout << "All players connected. Starting game." << std::endl;
            break;
        case GameState::PRE_FLOP:
            break;
        case GameState::POST_FLOP:
            break;
        case GameState::POST_TURN:
            break;
        case GameState::POST_RIVER:
            break;
        case GameState::SHOWDOWN:
            break;
        case GameState::POST_HAND:
            // TODO - needed ? 
            break;
        default:
            std::cerr << "Received packet with invalid game state!" << std::endl;
            break;
}