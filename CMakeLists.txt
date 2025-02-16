cmake_minimum_required(VERSION 3.16)
project(PokerApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source directories
set(SRC_DIR src)
set(CLIENT_DIR ${SRC_DIR}/client)
set(SERVER_DIR ${SRC_DIR}/server)

# Include directories
include_directories(${SRC_DIR})

# Collect source files
set(SRC_FILES 
    ${SRC_DIR}/network_manager.cpp
    ${SRC_DIR}/table.cpp
    ${SRC_DIR}/deck.cpp
    ${SRC_DIR}/player.cpp
)

set(CLIENT_FILES
    ${CLIENT_DIR}/main_client.cpp
    ${CLIENT_DIR}/poker_client.cpp
)

set(SERVER_FILES
    ${SERVER_DIR}/main_server.cpp
    ${SERVER_DIR}/poker_server.cpp
)

# Define client executable
add_executable(poker_client ${CLIENT_FILES} ${SRC_FILES})

# Define server executable
add_executable(poker_server ${SERVER_FILES} ${SRC_FILES})
