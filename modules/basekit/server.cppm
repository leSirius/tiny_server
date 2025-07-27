module;
#include <unistd.h>
export module Basekit :Server;
import  <cerrno>;
import  <functional>;
import <unordered_map>;
import Config;
import :Acceptor;
import :Socket;
import :InetAddress;
import :Connection;
import :EpollLoopChannel;

namespace basekit {
    export class Server {
    public:
        explicit Server(EventLoop *);

        // ~Server() = default;

        void newConnection(Socket *clientSock);

        void deleteConnection(const Socket *sock);

    private:
        EventLoop *loop{nullptr};
        unique_ptr<Acceptor> acceptor;
        unordered_map<int, Connection *> connections;
    };

    Server::Server(EventLoop *_loop) : loop(_loop) {
        acceptor = make_unique<Acceptor>(loop);
        acceptor->setNewConnectionCallback(
            [this](Socket *clientSock) { this->newConnection(clientSock); }
        );
    }

    void Server::newConnection(Socket *clientSock) {
        auto *conn = new Connection(loop, clientSock);
        conn->setDeleteCallback(
            [this](const Socket *sock) { this->deleteConnection(sock); }
        );
        connections[clientSock->getFd()] = conn;
    }

    void Server::deleteConnection(const Socket *sock) {
        const auto fd = sock->getFd();
        const Connection *conn = connections[fd];
        connections.erase(fd);
        delete conn;
    }
}
