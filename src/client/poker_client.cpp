#include "poker_client.h"

PokerClient::PokerClient() : network(), player_id(0) {}

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
                handle_player_action_update(socket, game_packet.action_update());
                break;
            case GamePacket::kDealerUpdate:
                handle_dealer_update(game_packet.dealer_update());
                break;
            case GamePacket::kHandResult:
                handle_hand_result_update(game_packet.hand_result());
                break;
            case GamePacket::kWaitingForAction:
                handle_waiting_for_action_message(game_packet.waiting_for_action());
                break;
            case GamePacket::kGameStartMessage:
                handle_game_start_message(game_packet.game_start_message());
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

void PokerClient::print_player_action_update(const PlayerActionUpdate& player_action_update) const {
    const PlayerDecision& player_decision = player_action_update.player_decision();

    std::cout << player_action_update.player_name();

    if(player_decision.player_action() == PlayerAction::CHECK_CALL) {
        if(player_decision.bet_size() == 0) {
            std::cout << " checked.\n";
        }
        else {
            std::cout << " called a bet of " << std::to_string(player_decision.bet_size()) << std::endl;
        }
    }
    else if(player_decision.player_action() == PlayerAction::RAISE) {
        std::cout << " raised to " << std::to_string(player_decision.bet_size()) << std::endl;
    }
    else {
        std::cout << " folded." << std::endl;
    }
}

PlayerDecision PokerClient::retrieve_action_from_player(const PlayerActionUpdate& player_action_update) const {
    // get betting action from the player
    int current_bet_size = player_action_update.player_decision().bet_size();
    std::string option_one = (current_bet_size == 0) ? "Check" : "Call";

    std::cout << "It is your turn to act." << std::endl; 
    std::cout << "The current bet size is " << std::to_string(current_bet_size) << std::endl;

    std::cout << "Your betting options are: \n";
    std::cout << "(1) " << option_one << "\n";
    std::cout << "(2) Raise\n";
    std::cout << "(3) Fold\n";

    PlayerAction betting_action;
    while(true) {
        std::cout << "\n Enter a number to indicate your decision: ";

        int action;
        std::cin >> action;

        if (std::cin.fail()) {
            std::cin.clear();  // Clear error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Discard invalid input
            std::cout << "Invalid input. Please enter a valid integer.\n";
            continue;
        }

        if(action != 1 && action != 2 && action != 3){
            std::cout << "Error: that is not an action option. Please enter a number representing a valid option." << std::endl;
            continue;
        }

        betting_action = static_cast<PlayerAction>(action-1);
        break;
    }
    
    // if raise, we need to know how much the player wants to raise
    int player_bet = (betting_action == PlayerAction::RAISE) ? current_bet_size : get_raise_amount(current_bet_size);

    PlayerDecision player_decision;
    player_decision.set_player_id(player_id);
    player_decision.set_player_action(betting_action);
    player_decision.set_bet_size(player_bet);

    return player_decision;
}

int PokerClient::get_raise_amount(int current_bet_size) const {
    int raise_amount = 0;
    while(true) {
        std::cout << "Note: if you raise more than is in your current stack you will be put all in." << std::endl;
        std::cout << "Enter the amount you want to raise to: ";

        std::cin >> raise_amount;

        if (std::cin.fail()) {
            std::cin.clear();  // Clear error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Discard invalid input
            std::cout << "Invalid input. Please enter a valid integer.\n";
            continue;
        }

        if (raise_amount < current_bet_size * 2) {
            std::cout << "Invalid input. You must double the previous bet at minimum." << std::endl;
            continue;
        }

        break;
    }

    return raise_amount;
}

void PokerClient::handle_player_stack_update(const PlayerStackUpdate& player_stack_update) {
    for(PlayerStack player : player_stack_update.players()) {
        std::cout << player.player_name() << "\n";
        std::cout << "Stack Size: " + std::to_string(player.stack_size()) << " Big Blinds\n" << std::endl;
    }
}

void PokerClient::handle_player_action_update(int socket, const PlayerActionUpdate& player_action_update) {
    print_player_action_update(player_action_update);

    if (!player_action_update.has_action()) {
        return;
    }

    // get player's action and send to server
    PlayerDecision player_decision = retrieve_action_from_player(player_action_update);
    network.send_to_server(socket, player_decision);
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

void PokerClient::handle_waiting_for_action_message(const WaitingForAction& waiting_for_action) {
    std::cout << "Waiting for action from " << std::to_string(waiting_for_action.player_id()) << "..." << std::endl;
}

void PokerClient::handle_game_start_message(const GameStartMessage& game_start_message) {
    player_id = game_start_message.player_id();

    std::cout << "All players connected. The game has started.\n";
}