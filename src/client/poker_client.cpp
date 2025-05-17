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
    std::cout << " ";
}

void PokerClient::print_hand(const HoleCards& hand) const {
    print_card(hand.first());
    print_card(hand.second());
}

void PokerClient::handle_player_stack_update(const PlayerStackUpdate& player_stack_update) {
    for(PlayerStack player : player_stack_update.players()) {
        std::cout << player.player_name() << "\n";
        std::cout << "Stack Size: " + std::to_string(player.stack_size()) << " Big Blinds\n" << std::endl;
    }
}

void PokerClient::handle_player_action_update(const PlayerActionUpdate& player_action_update) {
    return;
}

void PokerClient::handle_dealer_update(const DealerUpdate& dealer_update) {
    // TODO - optimization: reduce the number of states. can have just one state
    // that defines a card being dealt. POST_FLOP, POST_TURN, and POST_RIVER 
    // are very similar in functionality. 
    switch (dealer_update.game_state()) {
        case GameState::SET_UP:
            std::cout << "All players connected. Starting game." << std::endl;
            break;
        case GameState::PRE_FLOP:
            std::cout << "Small blind and big blind taken. Waiting for player to make betting action..." << std::endl;
            break;
        case GameState::POST_FLOP:
            print_board(dealer_update);
            std::cout << "The flop has been dealt. Waiting for player to make betting action..." << std::endl; 
            break;
        case GameState::POST_TURN:
            print_board(dealer_update);
            std::cout << "The turn has been dealt. Waiting for player to make betting action..." << std::endl; 
            break;
        case GameState::POST_RIVER:
            print_board(dealer_update);
            std::cout << "The river has been dealt. Waiting for player to make betting action..." << std::endl; 
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
}

void PokerClient::handle_hand_result_update(const HandResult& hand_result) {
    std::cout << "Total pot size: " << std::to_string(hand_result.pot_size()) << std::endl;
    if (hand_result.game_state() != GameState::SHOWDOWN) {
        std::cout << "Player " << hand_result.winners(0) << " won the hand." << std::endl;
        std::cout << "Awarded the whole pot" << std::endl;
        return;
    }

    int num_winners = hand_result.winners().size();
    std::string status = (num_winners == 1) ? " won " : " chopped ";
    std::string chips_won = (num_winners == 1) ? "the whole" : "1/" + std::to_string(num_winners);
    for (int winner = 0; winner < num_winners; ++winner) {
        std::cout << "Player " << std::to_string(hand_result.winners(winner)) << status << " the pot.\n";
        std::cout << "With the hand: ";
        print_hand(hand_result.player_hands(winner));
        std::cout << "\nAwarded " << chips_won << " the pot" << std::endl;
    }
}