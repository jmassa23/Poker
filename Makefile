# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -Werror -Wshadow -Wuninitialized

# Directories
SRC_DIR = src/
CLIENT_DIR = src/client
SERVER_DIR = src/server
OBJ_DIR = obj
BIN_DIR = bin

# File paths for the client
CLIENT_FILE = $(CLIENT_DIR)/main_client.cpp
CLIENT_OBJ = $(OBJ_DIR)/main_client.o
CLIENT_EXEC = $(BIN_DIR)/main_client

# Additional client source files (e.g., poker_client.cpp, network_manager.cpp)
CLIENT_SRC = $(CLIENT_DIR)/poker_client.cpp $(SRC_DIR)/network_manager.cpp
CLIENT_OBJ_FILES = $(OBJ_DIR)/main_client.o $(OBJ_DIR)/poker_client.o $(OBJ_DIR)/network_manager.o

# File paths for the server
SERVER_FILE = $(SERVER_DIR)/main_server.cpp
SERVER_OBJ = $(OBJ_DIR)/main_server.o
SERVER_EXEC = $(BIN_DIR)/main_server

# Additional server source files (e.g., poker_server.cpp, network_manager.cpp)
SERVER_SRC = $(SERVER_DIR)/poker_server.cpp $(SRC_DIR)/network_manager.cpp
SERVER_OBJ_FILES = $(SERVER_OBJ) $(OBJ_DIR)/poker_server.o $(OBJ_DIR)/network_manager.o

# Targets
all: $(CLIENT_EXEC) $(SERVER_EXEC)

client: $(CLIENT_EXEC)

server: $(SERVER_EXEC)

# Ensure the obj and bin directories exist before compilation
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Create the client executable
$(CLIENT_EXEC): $(CLIENT_OBJ_FILES) | $(OBJ_DIR) $(BIN_DIR)
	$(CXX) $(CLIENT_OBJ_FILES) -o $(CLIENT_EXEC)

# Create the server executable
$(SERVER_EXEC): $(SERVER_OBJ_FILES) | $(OBJ_DIR) $(BIN_DIR)
	$(CXX) $(SERVER_OBJ_FILES) -o $(SERVER_EXEC)

# Compile the source files for the client
$(OBJ_DIR)/main_client.o: $(CLIENT_FILE) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(CLIENT_FILE) -o $(OBJ_DIR)/main_client.o

# Compile the poker_client.cpp file for the client (if exists)
$(OBJ_DIR)/poker_client.o: $(CLIENT_DIR)/poker_client.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(CLIENT_DIR)/poker_client.cpp -o $(OBJ_DIR)/poker_client.o

# Compile the network_manager.cpp file for the client
$(OBJ_DIR)/network_manager.o: $(SRC_DIR)/network_manager.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/network_manager.cpp -o $(OBJ_DIR)/network_manager.o

# Compile the source files for the server
$(OBJ_DIR)/main_server.o: $(SERVER_FILE) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SERVER_FILE) -o $(OBJ_DIR)/main_server.o

# Compile the poker_server.cpp file for the server (if exists)
$(OBJ_DIR)/poker_server.o: $(SERVER_DIR)/poker_server.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SERVER_DIR)/poker_server.cpp -o $(OBJ_DIR)/poker_server.o

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
