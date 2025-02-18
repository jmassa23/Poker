#include "network_manager.h"

NetworkManager::NetworkManager() : _socket(-1) {}

NetworkManager::~NetworkManager() {
    if(_socket != -1){
        close(_socket);
    }
    for(int client_sock : _client_sockets){
        close(client_sock);
    }
}

bool NetworkManager::initialize_server() {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
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

    freeaddrinfo(servinfo);

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
    int new_socket = accept(_socket, their_addr, &sin_size);
    if (new_socket == -1) {
        perror("accept");
        return -1;
    }

    _client_sockets.push_back(new_socket);
    return new_socket;
}

void NetworkManager::set_up_client_sockets(const std::vector<int>& sockets) {
    _client_sockets = std::move(sockets);
}

std::vector<int> NetworkManager::get_client_sockets() const {
    return this->_client_sockets;
}

void NetworkManager::broadcast(const GamePacket& game_packet) {
    std::ranges::for_each(_client_sockets, [&game_packet](int client_socket){
        send_to_client(client_socket, game_packet);
    });
}

bool NetworkManager::send_to_client(int socket, const GamePacket& game_packet) {
    // first serialize the object
    std::string buffer;
    if(!game_packet.SerializeToString(&buffer)){
        std::cerr << "Unable to serialize game packet.";
        return false;
    }

   return send_serialized_data(socket, buffer);
}

bool NetworkManager::receive_from_client(int socket, PlayerDecision& player_decision) {
    std::vector<char> buffer;
    uint64_t data_size;
    if(!recv_serialized_data(socket, buffer, data_size)){
        return false;
    }

    return player_decision.ParseFromArray(buffer.data(), data_size);
}

// receive from client

int NetworkManager::connect_to_server(const char* ip_address) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
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

    freeaddrinfo(servinfo);

    return _socket;
}

bool NetworkManager::send_to_server(int socket, const PlayerDecision& player_decision) {
    // first serialize the object
    std::string buffer;
    if(!player_decision.SerializeToString(&buffer)){
        std::cerr << "Unable to serialize PlayerDecision object.";
        return false;
    }

    // then send the object 
    return send_serialized_data(socket, buffer);
}

bool NetworkManager::receive_from_server(int socket, GamePacket& game_packet) {
    std::vector<char> buffer;
    uint64_t data_size;
    if(!recv_serialized_data(socket, buffer, data_size)) {
        return false;
    }

    return game_packet.ParseFromArray(buffer.data(), data_size);
}

bool NetworkManager::send_all(int socket, char *data, int len) {
    return transfer_all_data<DataTransferType::SEND>(socket, data, len);
}

bool NetworkManager::recv_all(int socket, char* buffer, int len) {
    return transfer_all_data<DataTransferType::RECEIVE>(socket, buffer, len);
}

template <DataTransferType transferType>
typename std::enable_if<transferType == DataTransferType::SEND, int>::type
NetworkManager::transfer_data(int socket, char* data, int len, int flags) {
    return send(socket, data, len, flags);
}

template <DataTransferType transferType>
typename std::enable_if<transferType == DataTransferType::RECEIVE, int>::type
NetworkManager::transfer_data(int socket, char* data, int len, int flags) {
    return recv(socket, data, len, flags);
}

template <DataTransferType transferType>
bool NetworkManager::transfer_all_data(int socket, char* buffer, int len) {
    int total_bytes_transferred = 0;
    int transferred;

    while (total_bytes_transferred < len) {
        transferred = transfer_data<transferType>(socket, buffer + total_bytes_transferred, len - total_bytes_transferred, 0);

        if (transferred == -1) {
            return false;
        }

        total_bytes_transferred += transferred;
    }
    return true;
}

bool NetworkManager::send_serialized_data(int socket, std::string& data) {
    // send the size of the data
    uint64_t data_size = htonll(data.size());
    char data_size_buffer[sizeof(data_size)];
    memcpy(data_size_buffer, &data_size, sizeof(data_size));
    if(!send_all(socket, data_size_buffer, sizeof(data_size))){
        std::cerr << "Unable to send size of serialized PlayerDecision data.";
        return false;
    }

    // then send the object 
    return send_all(socket, data.data(), data.size());
}

bool NetworkManager::recv_serialized_data(int socket, std::vector<char>& buffer, uint64_t& data_size) {
    // receive the size of the object first
    if(!recv_all(socket, (char*)&data_size, sizeof(data_size))) {
        std::cerr << "Could not receive size of the GamePacket object size.";
        return false;
    }
    data_size = ntohll(data_size);

    // receive the object
    buffer = std::vector<char>(data_size);
    if(!recv_all(socket, buffer.data(), data_size)){
        std::cerr << "Could not receive GamePacket data.";
        return false;
    }

    return true;
}