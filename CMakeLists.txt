cmake_minimum_required(VERSION 3.31)
project(tiny_server_client)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common C++ flags for both executables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fmodules")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")

# Define common C++ module sources
set(COMMON_CXX_MODULES_SOURCES
        modules/utils.cppm
        modules/config.cppm
        modules/basekit/acceptor.cppm
        modules/basekit/basekit.cppm
        modules/basekit/buffer.cppm
        modules/basekit/connection.cppm
        modules/basekit/epoll_channel_loop.cppm
        modules/basekit/inet_address.cppm
        modules/basekit/server.cppm
        modules/basekit/socket.cppm
        modules/basekit/threadpool.cppm
)

# Define common private sources
set(COMMON_PRIVATE_SOURCES
        modules/basekit/epoll.cpp
        modules/basekit/eventloop.cpp
        modules/basekit/channel.cpp
)

# Tiny Server
add_executable(tiny_server main.cpp)
target_sources(tiny_server PUBLIC FILE_SET CXX_MODULES FILES ${COMMON_CXX_MODULES_SOURCES})
target_sources(tiny_server PRIVATE ${COMMON_PRIVATE_SOURCES})

# Tiny Client
add_executable(tiny_client client.cpp)
target_sources(tiny_client PUBLIC FILE_SET CXX_MODULES FILES ${COMMON_CXX_MODULES_SOURCES})
target_sources(tiny_client PRIVATE ${COMMON_PRIVATE_SOURCES})

# test server
add_executable(test test.cpp)
target_sources(test PUBLIC FILE_SET CXX_MODULES FILES ${COMMON_CXX_MODULES_SOURCES})
target_sources(test PRIVATE ${COMMON_PRIVATE_SOURCES})