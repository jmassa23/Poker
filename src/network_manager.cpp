#include "network_manager.h"

NetworkManager::NetworkManager() : _socket(-1) {}

NetworkManager::~NetworkManager() {
    if(_socket != -1){
        close(_socket);
    }
}

bool NetworkManager::initialize_server() {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rv;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    struct addrinfo * p;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((_socket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        int yes=1;
        if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            return false;
        }

        if (bind(_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(_socket);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return false;
    }

    // start listening for new client connections
    if (listen(_socket, 6) == -1) {
        perror("listen failed");
        return false;
    }

    _client_sockets.reserve(6);

    return true;
}

int NetworkManager::accept_client() {
    struct sockaddr *their_addr;
    socklen_t sin_size = sizeof(their_addr);
    int new_socket = accept(_socket, (struct sockaddr *)&their_addr, &sin_size);
    if (new_socket == -1) {
        perror("accept");
        return -1;
    }

    _client_sockets.push_back(new_socket);
    return new_socket;
}

int NetworkManager::connect_to_server(const char* ip_address) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(ip_address, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and connect to the first we can
    struct addrinfo * p;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((_socket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(_socket);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }

    freeaddrinfo(servinfo); // all done with this structure

    return _socket;
}