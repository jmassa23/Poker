#include "poker_server.h"

void handle_signal(int signum) {
    std::cout << "\nCaught signal " << signum << ", exiting gracefully...\n";
    std::exit(0);  // triggers network's destructor
}

PokerServer::PokerServer() : network() {}

PokerServer::~PokerServer() {}

void PokerServer::run() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

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

    table->send_game_start_message();
    
    // run the game
    while(true) {
        print_game_info(table);
        table->shuffle_deck();
        // TODO - play_hand should return which player(s) won the hand
        // and print this message to all players
        table->play_hand();
        table->update_dealer();
    }
}

void PokerServer::print_game_info(const PokerTable& table) const {
    // print stats about all players at the table including 
    // player's name, stack size, buy in, hands won (?)

    // print the players in the hand in the order they will act
    // note who the dealer is
    // print the board if cards have been dealt
}
