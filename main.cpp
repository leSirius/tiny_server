#include <sys/socket.h>

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
using namespace basekit;

class EchoServer {
public:
    EchoServer(string_view ip, int port);

    ~EchoServer() = default;

    void start() const;

    static void onConnect(const shared_ptr<ConnectionTCP> &conn);

    static void onMessage(const shared_ptr<ConnectionTCP> &conn);

private:
    ServerTCP server;
};

EchoServer::EchoServer(const string_view ip, const int port): server(ip, port) {
    server.setConnectCB([this](ConnectionTCP::CallbackParam conn) { this->onConnect(conn); });
    server.setMessageCB([this](ConnectionTCP::CallbackParam conn) { this->onMessage(conn); });
}

void EchoServer::start() const { server.start(); }

void EchoServer::onConnect(ConnectionTCP::CallbackParam conn) {
    const int clientFD = conn->getFD();
    InetAddress addr;
    getpeername(clientFD, addr.getReinterCC(), addr.getLenPtr());
    println(
        "thread {} [fd#{}] from {}:{}",
        currentThread::getTid(), clientFD, addr.getAddress(), addr.getPort()
    );
}

void EchoServer::onMessage(ConnectionTCP::CallbackParam conn) {
    if (conn->getState() == ConnectionTCP::State::Connected) {
        const auto msg = conn->getRecvContent();
        println("thread: {} Message from client: ", currentThread::getTid(), msg);
        conn->sendMsg(msg);
        conn->handleClose();
    }
}

int main(const int argc, char *argv[]) {
    const int port = argc == 2 ? stoi(argv[1]) : config::PORT;
    const EchoServer echo(config::ADDRESS, port);
    echo.start();
    return 0;
}
