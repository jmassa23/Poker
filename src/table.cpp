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

    // find the strongest hand rank amongst the remaining players
    for(int i=0; i<num_players; ++i) {
        int player_idx = remaining_players[i];
        hand_strengths[player_idx] = get_hand_strength(player_idx, community_cards);
        if(hand_strengths[i].hand_rank > max_rank) {
            max_rank = hand_strengths[i].hand_rank;
        }
    }
    
    // players with the strongest hand rank at the table get considered for tie break
    std::vector<int> eligible_to_win;
    for(auto& [player_idx, hand_info] : hand_strengths) {
        if(hand_info.hand_rank == max_rank) {
            eligible_to_win.push_back(player_idx);
        }
    }

    std::vector<int> winners = break_hand_rank_tie(eligible_to_win, hand_strengths);

    return winners;
}
    
void Table::award_chips_to_winner(int winner, int amount) {
    players_at_table[winner]->award_chips(amount);
}

void Table::award_chips_to_winners(const std::vector<int>& winners, int amount) {
    int num_winners = winners.size();
    int award_amount = amount / num_winners;
    int remaining_big_blinds = amount % num_winners;

    for(int winner : winners) {
        award_chips_to_winner(winner, award_amount);
        if(remaining_big_blinds > 0) {
            award_chips_to_winner(winner, 1);
            --remaining_big_blinds;
        }
    }
}

std::vector<int> Table::break_hand_rank_tie(std::vector<int> eligible_to_win, std::unordered_map<int, HandTieBreakInfo> hand_strengths) {
    int winner = eligible_to_win[0];
    int num_players = eligible_to_win.size();
    // TODO - possibly implement this caching optimization
    // winner -> set of players who had hands of equal or greater value
    // used to avoid calling found_equal_or_higher_value_hand function a second time
    //std::unordered_map<int, std::unordered_set<int>> cache;  
    // first find one player that either beats or ties every other player
    for(int i = 1; i < num_players; ++i) {
        if(found_equal_or_higher_value_hand(hand_strengths[winner], hand_strengths[eligible_to_win[i]])) {
            winner = eligible_to_win[i];
        }
    }

    // then find all players who tie with that player 
    std::vector<int> winners(1, winner);
    for(int i = 0; i < num_players; ++i) {
        if(eligible_to_win[i] == winner) {
            continue;
        }
        if(found_equal_or_higher_value_hand(hand_strengths[winner], hand_strengths[eligible_to_win[i]])) {
            winners.push_back(eligible_to_win[i]);
        }
    }

    return winners;
}

bool Table::found_equal_or_higher_value_hand(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    HandRank hand_rank = current_winner.hand_rank;
    if(hand_rank == HandRank::STRAIGHT_FLUSH || hand_rank == HandRank::FLUSH || hand_rank == HandRank::STRAIGHT
      || hand_rank == HandRank::HIGH_CARD) {
        return found_equal_or_higher_value_card(current_winner, challenger);
    }
    else if(hand_rank == HandRank::QUADS) {
        return found_equal_or_higher_value_quads(current_winner, challenger);
    }
    else if(hand_rank == HandRank::FULL_HOUSE) {
        return found_equal_or_higher_value_full_house(current_winner, challenger);
    }
    else if(hand_rank == HandRank::TRIPS) {
        return found_equal_or_higher_value_trips(current_winner, challenger);
    }
    else if(hand_rank == HandRank::TWO_PAIR) {
        return found_equal_or_higher_value_two_pair(current_winner, challenger);
    }
    else if(hand_rank == HandRank::PAIR) {
        return found_equal_or_higher_value_pair(current_winner, challenger);
    }

    std::cout << "ERROR: UNKNOWN ERROR OCCURRED. HAND MAY BE INVALID." << std::endl;
    return false;
}

bool Table::found_equal_or_higher_value_card(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    int num_cards = current_winner.indifferent_cards.size();
    for(int card_idx = 0; card_idx < num_cards; ++card_idx) {
        if(current_winner.indifferent_cards[card_idx].rank() > challenger.indifferent_cards[card_idx].rank()) {
            return false;
        }
        else if(current_winner.indifferent_cards[card_idx].rank() < challenger.indifferent_cards[card_idx].rank()) {
            return true;
        }
    }
    return true;
}

bool Table::found_equal_or_higher_value_quads(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    if(current_winner.quads_rank > challenger.quads_rank) {
        return true;
    }
    else if(current_winner.quads_rank < challenger.quads_rank) {
        return false;
    }
    return found_equal_or_higher_value_card(current_winner, challenger);
}

bool Table::found_equal_or_higher_value_full_house(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    if(found_equal_or_higher_value_trips(current_winner, challenger)) {
        return true;
    }
    return found_equal_or_higher_value_pair(current_winner, challenger);
}

bool Table::found_equal_or_higher_value_trips(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    if(current_winner.trips_rank > challenger.trips_rank) {
        return true;
    }
    else if(current_winner.trips_rank < challenger.trips_rank) {
        return false;
    }
    return found_equal_or_higher_value_card(current_winner, challenger);
}

bool Table::found_equal_or_higher_value_two_pair(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    // first pair
    if(current_winner.pair_ranks.first > challenger.pair_ranks.first) {
        return true;
    }
    else if(current_winner.pair_ranks.first < challenger.pair_ranks.first) {
        return false;
    }

    // second pair
    if(current_winner.pair_ranks.second > challenger.pair_ranks.second) {
        return true;
    }
    else if(current_winner.pair_ranks.second < challenger.pair_ranks.second) {
        return false;
    }

    return found_equal_or_higher_value_card(current_winner, challenger);
}

bool Table::found_equal_or_higher_value_pair(HandTieBreakInfo& current_winner, HandTieBreakInfo& challenger) {
    if(current_winner.pair_ranks.first > challenger.pair_ranks.first) {
        return true;
    }
    else if(current_winner.pair_ranks.first < challenger.pair_ranks.first) {
        return false;
    }
    return found_equal_or_higher_value_card(current_winner, challenger);
}

// TODO - add optimization to check for the strongest hand we've seen so far
// return a hand strength of -1 when our upper bound hand strength goes below current max
HandTieBreakInfo Table::get_hand_strength(int player_idx, const std::vector<Card>& community_cards) {
    // create combined hand
    std::vector<Card> combined_hand = build_combined_hand(player_idx, community_cards);

    HandTieBreakInfo result;
    determine_hand_strength(combined_hand, result);
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

void Table::determine_hand_strength(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info) {
    // will serve as lower bound for hand rank
    hand_info.hand_rank = HandRank::HIGH_CARD; // best hand rank we have so far
    HandRank upper_bound_hand_rank = HandRank::STRAIGHT_FLUSH; // hand rank we are currently checking for

    while(upper_bound_hand_rank > hand_info.hand_rank) {
        check_for_hand(upper_bound_hand_rank, hand_info, combined_cards);
        --upper_bound_hand_rank;
    }

    // depending on the rank of hand we have, store needed info about the five card hand
    get_additional_five_card_hand_data(combined_cards, hand_info);
}

void Table::check_for_hand(HandRank& upper_bound_hand_rank, HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards) {
    // dont need to check for flush, trips, two pair, or pair as we check in other functions
    if(upper_bound_hand_rank == HandRank::STRAIGHT_FLUSH) {
        check_for_straight_flush(hand_info, combined_cards);
    }
    else if(upper_bound_hand_rank == HandRank::QUADS) { 
        check_for_quads(hand_info, combined_cards);
    }
    else if(upper_bound_hand_rank == HandRank::FULL_HOUSE) {
        check_for_full_house(hand_info);
    }
    else if(upper_bound_hand_rank == HandRank::STRAIGHT) {
        check_for_straight(hand_info, combined_cards);
    }
}

void Table::check_for_straight_flush(HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards) {
    // check for flush 
    bool is_flush = false;
    Suit flush_suit = Suit::INVALID;
    std::vector<int> suits_count(4, 0);
    for(const Card& card : combined_cards) {
        if(++suits_count[card.suit()] == 5) {
            is_flush = true;
            flush_suit = card.suit();
            break;
        }
    }
    
    if(!is_flush) {
        return;
    }

    // new minimum possible hand rank is a flush
    hand_info.hand_rank = HandRank::FLUSH;

    // if we already have a flush, check if flush makes a straight also
    std::vector<Card> flush_cards;
    for(const Card& card : combined_cards) {
        if(card.suit() == flush_suit) {
            flush_cards.push_back(card);
        }
    }

    check_for_straight_given_flush(hand_info, flush_cards);
}

void Table::check_for_quads(HandTieBreakInfo& hand_info, const std::vector<Card>& combined_cards) {
    // we can check for all multi-rank hands (pair, two pair, trips, full house, quads)
    std::vector<int> card_rank_count(ACE_HIGH_RANK, 0);
    for(const Card& card : combined_cards) {
        card_rank_count[card.rank()]++;
    }

    for(int card_rank=card_rank_count.size()-1; card_rank>=0; --card_rank) {
        if(card_rank_count[card_rank] == 4 && hand_info.quads_rank == 0) {
            hand_info.hand_rank = HandRank::QUADS;
            hand_info.quads_rank = card_rank;
            return;
        }
        else if(card_rank_count[card_rank] == 3 && hand_info.trips_rank == 0) {
            update_hand_rank_if_necessary(HandRank::TRIPS, hand_info);
            hand_info.trips_rank = card_rank;
        }
        else if(card_rank_count[card_rank] == 2) {
            if(hand_info.pair_ranks.second > 0) { // already have highest possible pairs
                continue;
            }
            else if(hand_info.pair_ranks.first > 0) { // we have two pair
                update_hand_rank_if_necessary(HandRank::TWO_PAIR, hand_info);
                hand_info.pair_ranks.second = card_rank;
            }
            else { // we have a pair
                update_hand_rank_if_necessary(HandRank::PAIR, hand_info);
                hand_info.pair_ranks.first = card_rank;
            }
        }
    }
}

void Table::check_for_full_house(HandTieBreakInfo& hand_info) {
    // trivial as we already know if we have both trips and a pair
    if(hand_info.trips_rank > 0 && hand_info.pair_ranks.first > 0) {
        hand_info.hand_rank = HandRank::FULL_HOUSE;
    }
}

void Table::check_for_straight_given_flush(HandTieBreakInfo& hand_info, const std::vector<Card>& flush_cards) {
    Card high_card;
    if(check_for_straight(flush_cards, high_card)) {
        hand_info.hand_rank = HandRank::STRAIGHT_FLUSH;
        hand_info.indifferent_cards.reserve(1);
        hand_info.indifferent_cards.push_back(high_card);
    }
    else { // flush but not straight flush
        fill_n_highest_cards(flush_cards, hand_info, 5);
    }
}

void Table::check_for_straight(HandTieBreakInfo& hand_info, const std::vector<Card>& cards) {
    Card high_card;
    if(check_for_straight(cards, high_card)) {
        hand_info.hand_rank = HandRank::STRAIGHT;
        hand_info.indifferent_cards.reserve(1);
        hand_info.indifferent_cards.push_back(high_card);
    }
}

bool Table::check_for_straight(const std::vector<Card>& cards, Card& high_card) {
    int count = 1;
    high_card = cards[0];
    for(int i=1; i<cards.size(); ++i) {
        if(cards[i-1].rank() - cards[i].rank() == 1) {
            if(++count == 5) {
                return true;
            }
        }
        else if(cards[i-1].rank() == cards[i].rank()) {
            continue;
        }
        else {
            high_card = cards[i];
            count = 1;
        }
    }

    return false;
}

void Table::update_hand_rank_if_necessary(HandRank hand_rank, HandTieBreakInfo& hand_info) {
    if(hand_rank > hand_info.hand_rank) {
        hand_info.hand_rank = hand_rank;
    }
}

void Table::get_additional_five_card_hand_data(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info) {
    if(hand_info.hand_rank == HandRank::HIGH_CARD) {
        fill_n_highest_cards(combined_cards, hand_info, 5);
    }
    else if(hand_info.hand_rank == HandRank::PAIR) {
        fill_n_highest_cards(combined_cards, hand_info, 3);
    }
    else if(hand_info.hand_rank == HandRank::TRIPS) {
        fill_n_highest_cards(combined_cards, hand_info, 2);
    }
    else if(hand_info.hand_rank == HandRank::TWO_PAIR || hand_info.hand_rank == HandRank::QUADS) {
        fill_n_highest_cards(combined_cards, hand_info, 1);
    }
}

void Table::fill_n_highest_cards(const std::vector<Card>& combined_cards, HandTieBreakInfo& hand_info, int n) {
    hand_info.indifferent_cards.reserve(n);
    for(int i=0; i<n; ++i) {
        hand_info.indifferent_cards.push_back(combined_cards[i]);
    }
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