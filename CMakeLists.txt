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
set(BASEKIT_MODULES
        modules/basekit/basekit.cppm
        modules/basekit/tcp/acceptor.cppm
        modules/basekit/tcp/buffer.cppm
        modules/basekit/tcp/epoll_channel_loop.cppm
        modules/basekit/tcp/inetAddress.cppm
        modules/basekit/tcp/socket.cppm
        modules/basekit/tcp/tcpConnection.cppm
        modules/basekit/tcp/tcpServer.cppm
        modules/basekit/tcp/threadpool.cppm
        modules/basekit/tcp/eventloop_thread.cppm
        modules/basekit/tcp/eventloop_threadpool.cppm
        modules/basekit/timer/timestamp.cppm
        modules/basekit/timer/timerQueue.cppm
        modules/basekit/timer/countTimer.cppm
        modules/basekit/log/logStream.cppm
        modules/basekit/log/fixedBuffer.cppm
        modules/basekit/log/fmt.cppm
        modules/basekit/log/logger.cppm
        modules/basekit/log/asynclog.cppm
        modules/basekit/log/logfile.cppm
        modules/basekit/thread/current_thread.cppm

        modules/basekit/http/httpRequest.cppm
        #        modules/basekit/http/httpParser_discard.cppm
        modules/basekit/http/httpResponse.cppm
        modules/basekit/http/httpServer.cppm
        modules/basekit/http/httpParser.cppm
)

# Define common private sources
set(BASEKIT_SOURCES
        modules/basekit/tcp/epoll.cpp
        modules/basekit/tcp/eventloop.cpp
        modules/basekit/tcp/channel.cpp
        modules/basekit/thread/current_thread.cpp
        modules/basekit/timer/timerQueue.cpp
        modules/basekit/http/httpParser.cpp
)


# Tiny Server
add_executable(tcpServerTest test/tcpServerTest.cpp)
target_sources(tcpServerTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(tcpServerTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(tcpServerTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(tcpServerTest PRIVATE include)
target_include_directories(tcpServerTest PUBLIC include)

# Tiny Client
add_executable(tcpClientTest test/tcpClientTest.cpp)
target_sources(tcpClientTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(tcpClientTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(tcpClientTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(tcpClientTest PUBLIC include)

# test server
add_executable(testConnections test/testConnections.cpp)
target_sources(testConnections PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(testConnections PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(testConnections PRIVATE ${BASEKIT_SOURCES})
target_include_directories(testConnections PUBLIC include)

#http parser
add_executable(httpParseTest test/httpParseTest.cpp)
target_sources(httpParseTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(httpParseTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(httpParseTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(httpParseTest PUBLIC include)

# http server
add_executable(httpServerTest test/httpServerTest.cpp)
target_sources(httpServerTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(httpServerTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(httpServerTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(httpServerTest PUBLIC include)

# http client
add_executable(httpClientTest test/httpClientTest.cpp)
target_sources(httpClientTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(httpClientTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(httpClientTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(httpClientTest PUBLIC include)

add_executable(logBufferTest test/logBufferTest.cpp)
target_sources(logBufferTest PUBLIC FILE_SET CXX_MODULES FILES ${GENERAL_MODULES})
target_sources(logBufferTest PUBLIC FILE_SET CXX_MODULES FILES ${BASEKIT_MODULES})
target_sources(logBufferTest PRIVATE ${BASEKIT_SOURCES})
target_include_directories(logBufferTest PRIVATE include)