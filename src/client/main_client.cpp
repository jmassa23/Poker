#include "poker_client.h"

int main([[maybe_unused]] int argc, char* argv[]){
    PokerClient client;
    client.run(
        (argc > 1) ? argv[1] : ""
    );
}