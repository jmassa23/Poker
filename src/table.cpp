#include "table.h"

Table::Table(const std::vector<int>& players) :
              deck(std::make_unique<Deck>())
            , current_game_state(GameState::SET_UP)
            , current_dealer(0)
{
    // randomly sit the players at the table
    std::random_device rd;
    std::mt19937 generator(rd());
    
    players_at_table = build_players(players, generator);

    player_sockets = std::move(players);
}

PlayerList Table::build_players(const std::vector<int>& players, std::mt19937& generator) {
    PlayerList built_list;
    built_list.reserve(players.size());
    
    // used for token generation
    std::unordered_set<uint64_t> unique_tokens;
    std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);

    int i=0;
    // for each player, generate a new token
    for(const auto& player_socket : players){
        uint64_t token = generate_token(unique_tokens, dist, generator);
        auto new_player_ptr = std::make_shared<Player>("Player " + std::to_string(i++), token, 500);
        socket_to_player[player_socket] = new_player_ptr;
        // TODO - better way to initialize player name and buy_in
        built_list.emplace_back(std::move(new_player_ptr));
    }

    std::ranges::shuffle(built_list, generator);
    return std::move(built_list);
}

void Table::deal_hand(int& deck_idx) {
    int num_players = players_at_table.size();

    // deal first card
    for(int offset = 1; offset <= num_players; ++offset) {
        int player_id = (current_dealer + offset) % num_players;
        players_at_table[player_id]->deal_card(deck->get_card(deck_idx++), false);
    }

    // deal second card
    for(int offset = 1; offset <= num_players; ++offset) {
        int player_id = (current_dealer + offset) % num_players;
        players_at_table[player_id]->deal_card(deck->get_card(deck_idx++), true);
    }
}

uint64_t Table::generate_token(std::unordered_set<uint64_t>& unique_tokens
            , std::uniform_int_distribution<uint64_t>& dist
            , std::mt19937& generator) const 
{
    // generate a unique token for each player at the table
    uint64_t token;
    do {
        token = dist(generator);
    } while (unique_tokens.count(token)); // ensure each token is unique

    unique_tokens.insert(token);
    return token;
}

void Table::shuffle_deck() {
    deck->shuffle();
}

void Table::play_hand() {
    // TODO  - allow stack sizes of 0 to buy back in 
            // handle players who are excluded bc they have a stack size of 0
    int num_players = players_at_table.size();

    int pot_size = 0; // in big blinds
    int deck_idx = 0;
    std::unordered_set<int> excluded_players; // players no longer in the hand

    int current_player_action = current_dealer;
    deal_hand(deck_idx);
    take_blinds(current_player_action, excluded_players);
    
}

void Table::take_blinds(int& player_idx, std::unordered_set<int>& excluded_players) {
    update_player_idx(player_idx, excluded_players);
    players_at_table[player_idx]->take_small_blind();
    update_player_idx(player_idx, excluded_players);
    players_at_table[player_idx]->take_big_blind();
    update_player_idx(player_idx, excluded_players);
}

void Table::update_player_idx(int& player_idx, std::unordered_set<int>& excluded_players) {
    while(!excluded_players.contains(player_idx++)) {}
}

void Table::update_dealer() {
    int n = players_at_table.size();
    current_dealer = (current_dealer + 1) % n;
}

void Table::broadcast_to_players(const GamePacket& game_packet) const {
    NetworkManager::broadcast(game_packet, player_sockets);
}