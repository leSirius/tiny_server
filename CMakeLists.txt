cmake_minimum_required(VERSION 3.31)
project(tiny_server_client)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common C++ flags for both executables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fmodules")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")

set(GENERAL_MODULES
        modules/config.cppm
        modules/utils.cppm
)

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

        modules/tcp/timer/timestamp.cppm
        modules/tcp/timer/timerQueue.cppm
        modules/tcp/timer/countTimer.cppm

        modules/log/log.cppm
        modules/log/logStream.cppm
        modules/log/fixedBuffer.cppm
        modules/log/fmt.cppm
        modules/log/logger.cppm
)

# Define common private sources
set(TCP_SOURCES
        modules/tcp/epoll.cpp
        modules/tcp/eventloop.cpp
        modules/tcp/channel.cpp
        modules/tcp/current_thread.cpp

        modules/tcp/timer/timerQueue.cpp
)

set(HTTP_MODULES
        modules/http/http.cppm
        modules/http/httpRequest.cppm
        modules/http/httpParser.cppm
        modules/http/httpResponse.cppm
        modules/http/httpServer.cppm
)

#set(LOG_MODULES
#        ${TCP_MODULES}
#        modules/log/log.cppm
#        modules/log/logStream.cppm
#        modules/log/fixedBuffer.cppm
#        modules/log/fmt.cppm
#        modules/log/logger.cppm
#)
#
#set(LOG_SOURCES
#        ${TCP_SOURCES}
#)


# Tiny Server
add_executable(tiny_server test/tiny_server.cpp)
target_sources(tiny_server PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(tiny_server PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(tiny_server PRIVATE ${TCP_SOURCES})
target_include_directories(tiny_server PRIVATE include)
target_include_directories(tiny_server PUBLIC include)


# Tiny Client
add_executable(tiny_client test/tiny_client.cpp)
target_sources(tiny_client PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(tiny_client PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(tiny_client PRIVATE ${TCP_SOURCES})
target_include_directories(tiny_client PUBLIC include)


# test server
add_executable(test_tcp test/test.cpp)
target_sources(test_tcp PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(test_tcp PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(test_tcp PRIVATE ${TCP_SOURCES})
target_include_directories(test_tcp PUBLIC include)


#http parser
add_executable(test_http_parse test/testHttpParser.cpp)
target_sources(test_http_parse PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(test_http_parse PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(test_http_parse PRIVATE ${TCP_SOURCES})
target_sources(test_http_parse PUBLIC FILE_SET CXX_MODULES FILES ${HTTP_MODULES})
target_sources(test_http_parse PRIVATE ${HTTP_SOURCES})
target_include_directories(test_http_parse PUBLIC include)


# http server
add_executable(http_server test/http_server.cpp)
target_sources(http_server PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(http_server PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(http_server PRIVATE ${TCP_SOURCES})
target_sources(http_server PUBLIC FILE_SET CXX_MODULES FILES ${HTTP_MODULES})
target_sources(http_server PRIVATE ${HTTP_SOURCES})
target_include_directories(http_server PUBLIC include)

# http client
add_executable(http_client test/http_client.cpp)
target_sources(http_client PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(http_client PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(http_client PRIVATE ${TCP_SOURCES})
target_sources(http_client PUBLIC FILE_SET CXX_MODULES FILES ${HTTP_MODULES})
target_sources(http_client PRIVATE ${HTTP_SOURCES})
target_include_directories(http_client PUBLIC include)

add_executable(logBuffer test/logBufferTest.cpp)
target_sources(logBuffer PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(logBuffer PUBLIC FILE_SET CXX_MODULES FILES ${TCP_MODULES})
target_sources(logBuffer PRIVATE ${TCP_SOURCES})
target_include_directories(logBuffer PRIVATE include)