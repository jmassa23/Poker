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

uint64_t Table::generate_token(std::unordered_set<uint64_t>& unique_tokens
            , std::uniform_int_distribution<uint64_t>& dist
            , std::mt19937& generator) const 
{
    // generate a unique token for each player at the table
    uint64_t token;
    do {
        token = dist(generator);
    } while (unique_tokens.contains(token)); // ensure each token is unique

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
    deal_hands(deck_idx);
    take_blinds(current_player_action, excluded_players);

    std::vector<Card> community_cards;
    community_cards.reserve(MAX_NUMBER_OF_COMMUNITY_CARDS);

    int winner = -1; // indicates which player won the hand, if decided by betting.

    // handle pre flop betting action then deal flop and reset player action
    if((winner = handle_betting_action(true, excluded_players, current_player_action, pot_size, deck_idx)) > -1) {
        award_chips_to_winner(winner, pot_size);
        return;
    }
    deal_flop(deck_idx, community_cards);

    // handle post flop betting action then deal turn
    if((winner = handle_betting_action(false, excluded_players, current_player_action, pot_size, deck_idx)) > -1) {
        award_chips_to_winner(winner, pot_size);
        return;
    }
    deal_turn_or_river(deck_idx, community_cards);

    // handle post turn betting action then deal river
    if((winner = handle_betting_action(false, excluded_players, current_player_action, pot_size, deck_idx)) > -1) {
        award_chips_to_winner(winner, pot_size);
        return;
    }
    deal_turn_or_river(deck_idx, community_cards);

    // handle post river action
    if((winner = handle_betting_action(false, excluded_players, current_player_action, pot_size, deck_idx)) > -1) {
        award_chips_to_winner(winner, pot_size);
        return;
    }

    // decide winner (contains multiple if a draw)
    std::vector<int> remaining_players = get_remaining_players(excluded_players);
    std::vector<int> winners = decide_winners(remaining_players, community_cards);

    // award winner chips
    award_chips_to_winners(winners, pot_size);
}

void Table::deal_hands(int& deck_idx) {
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

void Table::deal_community_card(int& deck_idx, std::vector<Card>& community_cards) {
    community_cards.emplace_back(std::move(deck->get_card(deck_idx++)));
}

void Table::deal_flop(int& deck_idx, std::vector<Card>& community_cards) {
    // burn a card and deal three cards
    ++deck_idx;
    deal_community_card(deck_idx, community_cards);
    deal_community_card(deck_idx, community_cards);
    deal_community_card(deck_idx, community_cards);
}

void Table::deal_turn_or_river(int& deck_idx, std::vector<Card>& community_cards) {
    // burn a card and deal a card
    ++deck_idx;
    deal_community_card(deck_idx, community_cards);
}

// update the current player to be the BB+1 and take the blinds from SB and BB
void Table::take_blinds(int& player_idx, std::unordered_set<int>& excluded_players) {
    update_player_idx(player_idx, excluded_players);
    players_at_table[player_idx]->take_small_blind();
    update_player_idx(player_idx, excluded_players);
    players_at_table[player_idx]->take_big_blind();
    update_player_idx(player_idx, excluded_players);
}

// returns a nonnegative number when the winner has already been decided by betting
// returns -1 otherwise
int Table::handle_betting_action(bool is_pre_flop, std::unordered_set<int>& excluded_players, int& current_player_action, int& pot_size, int& deck_idx) {
    return -1;
}

std::vector<int> Table::decide_winners(const std::vector<int>& remaining_players, const std::vector<Card>& community_cards) {
    int num_players = remaining_players.size();
    std::unordered_map<int, HandTieBreakInfo> hand_strengths(num_players);
    HandRank max_rank = HandRank::HIGH_CARD;
    for(int i=0; i<num_players; ++i){
        int player_idx = remaining_players[i];
        hand_strengths[player_idx] = determine_hand_strength(player_idx, community_cards);
        if(hand_strengths[i].hand_rank > max_rank) {
            max_rank = hand_strengths[i].hand_rank;
        }
    }

    std::vector<int> winners;
    for(auto& [player_idx, handInfo] : hand_strengths) {
        if(handInfo.hand_rank == max_rank) {
            winners.push_back(player_idx);
        }   
    }

    return winners;
}
    
void Table::award_chips_to_winner(int winner, int amount) {
    players_at_table[winner]->award_chips(amount);
}

void Table::award_chips_to_winners(const std::vector<int>& winners, int amount) {
    int num_winners = winners.size();
    int remaining_big_blinds = amount % num_winners;

    for(int winner : winners) {
        award_chips_to_winner(winner, amount / num_winners);
        if(remaining_big_blinds > 0) {
            award_chips_to_winner(winner, 1);
            --remaining_big_blinds;
        }
    }
}

// TODO - add optimization to check for the strongest hand we've seen so far
// return a hand strength of -1 when our upper bound hand strength goes below current max
HandTieBreakInfo Table::determine_hand_strength(int player_idx, const std::vector<Card>& community_cards) {
    // create combined hand
    std::vector<Card> combined_hand = build_combined_hand(player_idx, community_cards);

    HandTieBreakInfo result;
    return result;
}

std::vector<Card> Table::build_combined_hand(int player_idx, const std::vector<Card>& community_cards) {
    Hand hole_cards = players_at_table[player_idx]->get_hand();
    
    std::vector<Card> combined_hand;
    combined_hand.reserve(8);
    combined_hand.push_back(hole_cards.first);
    combined_hand.push_back(hole_cards.second);
    combined_hand.insert(combined_hand.end(), community_cards.begin(), community_cards.end());

    // sort cards by descending rank
    std::sort(combined_hand.begin(), combined_hand.end(), [](const Card& a, const Card& b){
        return a.rank() > b.rank();
    });

    // if we have an ace, add a card with rank 1 to end of combined hand 
    // used to check for the wheel
    if(combined_hand[0].rank() == ACE_HIGH_RANK) {
        combined_hand.emplace_back();
        combined_hand.back().set_rank(ACE_LOW_RANK);
        combined_hand.back().set_suit(combined_hand[0].suit());
    }

    return combined_hand;
}

void Table::update_player_idx(int& player_idx, std::unordered_set<int>& excluded_players) {
    while(!excluded_players.contains(player_idx++)) {}
}

std::vector<int> Table::get_remaining_players(const std::unordered_set<int> excluded_players) {
    int num_players = players_at_table.size();
    std::vector<int> result;
    result.reserve(num_players - excluded_players.size());

    for(int player_idx=0; player_idx<num_players; ++player_idx) {
        if(!excluded_players.contains(player_idx)) {
            result.push_back(player_idx);
        }
    }

    return result;
}

void Table::update_dealer() {
    int n = players_at_table.size();
    current_dealer = (current_dealer + 1) % n;
}

void Table::broadcast_to_players(const GamePacket& game_packet) const {
    NetworkManager::broadcast(game_packet, player_sockets);
}