#include "table.h"

Table::Table(const std::vector<int>& players) :
              deck(std::make_unique<Deck>())
            , current_game_state(GameState::PRE_FLOP)
            , current_dealer(0)
{
    // shuffle the players at the table
    std::random_device rd;
    std::mt19937 generator(rd());
    
    players_at_table = build_players(players, generator);
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