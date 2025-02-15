#include "table.h"

Table::Table(const std::vector<int>& players) :
            players_at_table(players)
            , deck(std::make_unique<Deck>())
            , current_game_state(GameState::PRE_FLOP)
            , current_dealer(0)
{
    // shuffle the players at the table
    std::random_device rd;
    std::mt19937 g(rd());
    std::ranges::shuffle(players_at_table, g);
}