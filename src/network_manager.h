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

class NetworkManager {
public:
    bool initialize();
    
private:
    int server_socket;
    std::vector<int> client_sockets;
};