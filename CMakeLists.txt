cmake_minimum_required(VERSION 3.31)
project(tiny_server_client)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common C++ flags for both executables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fmodules")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")

# Define common C++ module sources
set(TCP_MODULES
        modules/tcp/acceptor.cppm
        modules/tcp/tcp.cppm
        modules/tcp/buffer.cppm
        modules/tcp/epoll_channel_loop.cppm
        modules/tcp/inet_address.cppm
        modules/tcp/socket.cppm
        modules/tcp/tcp_connection.cppm
        modules/tcp/tcp_server.cppm
        modules/tcp/threadpool.cppm
        modules/tcp/current_thread.cppm
        modules/tcp/eventloop_thread.cppm
        modules/tcp/eventloop_threadpool.cppm
        modules/config.cppm
        modules/utils.cppm
)

# Define common private sources
set(TCP_SOURCES
        modules/tcp/epoll.cpp
        modules/tcp/eventloop.cpp
        modules/tcp/channel.cpp
        modules/tcp/current_thread.cpp
)

set(HTTP_MODULES
        modules/http/http.cppm
        modules/http/httpRequest.cppm
        modules/http/httpParser.cppm
)
set(HTTP_SOURCES
        modules/http/httpParser.cpp
)

add_executable(test_http test/test_http.cpp)
target_sources(test_http PUBLIC FILE_SET CXX_MODULES FILES ${HTTP_MODULES})
target_sources(test_http PRIVATE ${HTTP_SOURCES})

# Tiny Server
add_executable(tiny_server test/tiny_server.cpp)
target_sources(tiny_server PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(tiny_server PRIVATE ${TCP_SOURCES})

# Tiny Client
add_executable(tiny_client test/tiny_client.cpp)
target_sources(tiny_client PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(tiny_client PRIVATE ${TCP_SOURCES})

# test server
add_executable(test test/test.cpp)
target_sources(test PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(test PRIVATE ${TCP_SOURCES})