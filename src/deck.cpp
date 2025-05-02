#include "deck.h"

Deck::Deck() : gen(std::random_device{}()) {
    // build the deck
    deck.reserve(DECK_SIZE);

    std::vector<Suit> suits = {Suit::SPADE, Suit::HEART, Suit::CLUB, Suit::DIAMOND};
    auto ranks = std::views::iota(2, 15);

    // Use ranges to generate the deck
    for (const auto& suit : suits) {    
        std::ranges::for_each(ranks, [&](int rank) {
            deck.emplace_back();
            deck.back().set_rank(rank);
            deck.back().set_suit(suit);
        });
    }
}

void Deck::shuffle() {
    std::ranges::shuffle(deck, gen);
}

Card Deck::get_card(int index) {
    return deck[index];
}