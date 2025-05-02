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

    std::cout << "All players connected. Starting game." << std::endl;

    // start the game.
    play_game(players);
}

void PokerServer::play_game(const std::vector<int>& client_sockets) const {
    // Create a table and run the game
    PokerTable table = std::make_unique<Table>(client_sockets);

    GamePacket game_packet;
    create_game_start_message(game_packet);
    table->broadcast_to_players(game_packet);
    
    // run the game
    while(true) {
        print_game_info(table);
        table->shuffle_deck();
        table->play_hand();
        table->update_dealer();
    }
}

void PokerServer::create_game_start_message(GamePacket& game_packet) const {
    game_packet.set_game_state(GameState::SET_UP);
    Card* card1 = game_packet.add_board();
    card1->set_rank(14);
    card1->set_suit(Suit::SPADE);

    Card* card2 = game_packet.add_board();
    card2->set_rank(13);
    card2->set_suit(Suit::DIAMOND);

    Card* card3 = game_packet.add_board();
    card3->set_rank(10);
    card3->set_suit(Suit::HEART);

    Card* card4 = game_packet.add_board();
    card4->set_rank(9);
    card4->set_suit(Suit::CLUB);

    Card* card5 = game_packet.add_board();
    card5->set_rank(11);
    card5->set_suit(Suit::HEART);
}

void PokerServer::print_game_info(const PokerTable& table) const {
    // print stats about all players at the table including 
    // player's name, stack size, buy in, hands won (?)

    // print the players in the hand in the order they will act
    // note who the dealer is
    // print the board if cards have been dealt
}
