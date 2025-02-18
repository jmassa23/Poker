cmake_minimum_required(VERSION 3.16)
project(PokerApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")

# Find Protobuf (static linking)
find_package(Protobuf REQUIRED)
set(Protobuf_STATIC ON)  # Make sure Protobuf is statically linked

# Include directories
include_directories(${Protobuf_INCLUDE_DIRS})

# Define source directories
set(SRC_DIR src)
set(CLIENT_DIR ${SRC_DIR}/client)
set(SERVER_DIR ${SRC_DIR}/server)

include_directories(${SRC_DIR})

# Define protobuf files
set(PROTO_FILES proto/game_packet.proto)

# Generate protobuf files (.cc and .h)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
message(STATUS "Generated protobuf files: ${PROTO_SRCS} ${PROTO_HDRS}")
message(STATUS "Protobuf include dir: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")

# Add the generated protobuf files' directory to the include path
include_directories(${CMAKE_BINARY_DIR})  # Add the build directory to include directories

# Collect source files
set(SRC_FILES 
    ${SRC_DIR}/network_manager.cpp
    ${SRC_DIR}/table.cpp
    ${SRC_DIR}/deck.cpp
    ${SRC_DIR}/player.cpp
    ${PROTO_SRCS}  # Add generated protobuf .cc files
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

# Link Protobuf statically
target_link_libraries(poker_client PRIVATE ${Protobuf_LIBRARIES})
target_link_libraries(poker_server PRIVATE ${Protobuf_LIBRARIES})

# Remove static library flags for macOS, as they are not necessary
if(APPLE)
    set_target_properties(poker_client PROPERTIES LINK_FLAGS "")
    set_target_properties(poker_server PROPERTIES LINK_FLAGS "")
endif()
