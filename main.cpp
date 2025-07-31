#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

import <array>;
import <csignal>;
import <functional>;
import <iostream>;
import <string_view>;
import <vector>;

import config;
import utils;
import basekit;

using namespace std;

int main() {
    using namespace basekit;
    ServerTCP server(config::ADDRESS, config::PORT);
    server.setMessageCB([](ConnectionTCP *conn) {
        const auto recvMsg = conn->getReadBuffer()->c_str();
        println("Message from clientID {} is {}", conn->getID(), recvMsg);
        conn->sendMsg(recvMsg);
    });
    server.start();
    return 0;
}

// int main() {
//     using namespace basekit;
//
//     EventLoop loop{};
//     ServerTCP server(&loop);
//     server.setConnectionCB([](ConnectionTCP *conn) {
//         conn->connRead();
//         if (conn->getState() == ConnectionTCP::State::Closed) {
//             conn->handleClose();
//             return;
//         }
//         println("Message from client {} : {}", conn->getSocket()->getFd(), conn->readBufferContent());
//         conn->setSendBuffer(conn->readBufferContent());
//         conn->connWrite();
//     });
//
//     loop.loop();
//     return 0;
// }
