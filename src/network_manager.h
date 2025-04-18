#pragma once

#include <sys/types.h>    // Defines data types used in socket programming
#include <sys/socket.h>   // Core socket functions (socket, bind, listen, accept, etc.)
#include <netinet/in.h>   // Structures for internet domain addresses (sockaddr_in)
#include <arpa/inet.h>    // Functions for IP address conversion (inet_pton, inet_ntop)
#include <unistd.h>       // For close() function to close sockets
#include <netdb.h>        // getaddrinfo(), gethostbyname(), etc.
#include <cstring>        // memset() and other string manipulation functions
#include <errno.h>        // Error handling (errno)
#include <stdio.h>        // printf, perror
#include <stdlib.h>       // exit()

#include <vector>
#include <string>
#include <type_traits>

#include "utils.h"
#include "game_packet.pb.h"

#define PORT "7777"

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // ----- SERVER SIDE -----

    bool initialize_server();
    int accept_client();
    // TODO - void set_up_client_sockets(const std::vector<int>& sockets);
    // std::vector<int> get_client_sockets() const;
    // send game updates from server to all client sockets
    static void broadcast(const GamePacket& game_packet, const std::vector<int>& client_sockets);
    static bool send_to_client(int socket, const GamePacket& game_packet);
    static bool receive_from_client(int socket, PlayerDecision& player_decision);


    // ----- CLIENT SIDE -----

    int connect_to_server(const char* ip_address);
    static bool send_to_server(int socket, const PlayerDecision& player_decision);
    static bool receive_from_server(int socket, GamePacket& game_packet);


    // ----- COMMON FUNCTIONS -----
    
    int get_port() const; // TODO - do we need this ? 
    static bool send_serialized_data(int socket, std::string& data);
    static bool recv_serialized_data(int socket, std::vector<char>& buffer, uint64_t& data_size);
    static bool send_all(int socket, char* data, int len);
    static bool recv_all(int socket, char* buffer, int len);

    template <DataTransferType transferType>
    static bool transfer_all_data(int socket, char* data, int len);

    template <DataTransferType transferType>
    static typename std::enable_if<transferType == DataTransferType::SEND, int>::type
    transfer_data(int socket, char* data, int len, int flags);

    template <DataTransferType transferType>
    static typename std::enable_if<transferType == DataTransferType::RECEIVE, int>::type
    transfer_data(int socket, char* data, int len, int flags);

private:
    int _socket;
    std::vector<int> _client_sockets;
};