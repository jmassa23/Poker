#pragma once

#include <vector>
#include <utility>
#include <iostream>

#include "game_packet.pb.h"

// ----- ENUMS -----

// enum class GameState {
//     SET_UP,
//     PRE_FLOP,
//     POST_FLOP,
//     POST_TURN,
//     POST_RIVER,
//     POST_HAND // award money to winner(s)
// };

// enum class PlayerAction {
//     CHECK_CALL,
//     RAISE,
//     FOLD
// };

enum class HandRank {
    HIGH_CARD, // value of 0
    PAIR,
    TWO_PAIR,
    TRIPS,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    QUADS,
    STRAIGHT_FLUSH // value of 8
};

// enum class Suit {
//     SPADE,
//     HEART,
//     CLUB,
//     DIAMOND,
//     INVALID
// };

// ----- STRUCTS -----

// struct Card {
//     // value from 2 to 14 (Ace)
//     // can also be a 1 to represent a low ace when checking for the wheel (lowest straight)
//     int rank;
//     Suit suit;

//     Card(int _rank, Suit _suit) : rank(_rank), suit(_suit) {}
//     Card() : rank(0), suit(Suit::INVALID) {}

//     void print_card() const {
//         std::cout << rank;
//         switch(suit) {
//             case Suit::SPADE:
//                 std::cout << "\u2660";
//                 break;
//             case Suit::HEART:
//                 std::cout << "\u2665";
//                 break;
//             case Suit::CLUB:
//                 std::cout << "\u2663";
//                 break;
//             case Suit::DIAMOND:
//                 std::cout << "\u2666";
//                 break;
//             default:
//                 std::cout << "ERROR: This text should never print." << std::endl;
//                 break;
//         }
//     }
// };

struct PokerHand {
    std::vector<Card> indifferent_cards;
    std::pair<int, int> pair_ranks;
    HandRank hand_rank;
    int trips_rank;
    int quads_rank;
};

// // sent by the server to the client
// struct GamePacket {
//     std::vector<Card> board;
//     GameState game_state;
//     int award_amount;
//     int player_id;
// };

// // send by the client to the server
// struct PlayerDecision {
//     int player_id;
//     PlayerAction action;
//     int bet_size; // 0 for check or fold, greater than 0 otherwise
// };