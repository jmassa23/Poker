#include "deck.h"

Deck::Deck() : gen(std::random_device{}()) {
    // build the deck
    deck.reserve(52);

    std::vector<Suit> suits = {Suit::SPADE, Suit::HEART, Suit::CLUB, Suit::DIAMOND};
    auto ranks = std::views::iota(2, 15);

    // Use ranges to generate the deck
    for (auto suit : suits) {    
        std::ranges::for_each(ranks, [&](int rank) {
            deck.emplace_back(rank, suit);
        });
    }
}


void Deck::shuffle() {
    std::ranges::shuffle(deck, gen);
}