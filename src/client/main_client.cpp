#include "poker_client.h"

int main(int argc, char* argv[]){
    PokerClient client;
    client.run(argv[1]);

    std::cout << "There are " << argc << " arguments." << std::endl;
}