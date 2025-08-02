module;
#include <cassert>
#include <unistd.h>
export module basekit :tcpServer;
import  <cerrno>;
import  <functional>;
import  <map>;
import <unordered_map>;

import config;
import :acceptor;
import :socket;
import :inetAddress;
import :tcpConnection;
import :epollLoopChannel;
import :currentThread;

namespace basekit {
    export class ServerTCP {
    public:
        ServerTCP(string_view IP, int port);

        ~ServerTCP();

        void start() const;

        // explicit TCPServer(EventLoop *);

        void handleNewConnect(int fd);

        void handleClose(ConnectionTCP::CallbackParam conn);

        void handleCloseInLoop(ConnectionTCP::CallbackParam conn);

        void setConnectCB(ConnectionTCP::CallBackType fn);

        void setMessageCB(ConnectionTCP::CallBackType fn);

    private:
        unique_ptr<EventLoop> mainReactor{nullptr};
        vector<unique_ptr<EventLoop> > subReactors;
        unique_ptr<Acceptor> acceptor;
        unordered_map<int, shared_ptr<ConnectionTCP> > connectionMap{};
        unique_ptr<ThreadPool> threadPool{};
        ConnectionTCP::CallBackType onConnectCB;
        ConnectionTCP::CallBackType onMessageCB;
        int nextConnID{1};
    };

    ServerTCP::ServerTCP(const string_view IP, const int port) {
        mainReactor = make_unique<EventLoop>();
        acceptor = make_unique<Acceptor>(mainReactor.get(), IP, port);
        acceptor->setNewConnectCB([this](const int fd) { this->handleNewConnect(fd); });
        const auto cores = config::CPU_CORES;
        threadPool = std::make_unique<ThreadPool>(cores);
        for (size_t i = 0; i < cores; ++i) {
            auto sub_reactor = make_unique<EventLoop>();
            subReactors.push_back(std::move(sub_reactor));
        }
    }

    ServerTCP::~ServerTCP() = default;

    void ServerTCP::start() const {
        for (auto &reactor: subReactors) {
            threadPool->add([r = reactor.get()] { r->loop(); });
        }
        mainReactor->loop();
    }

    void ServerTCP::handleNewConnect(const int fd) {
        assert(fd != -1);
        const auto rand = fd % subReactors.size();
        auto conn = make_shared<ConnectionTCP>(nextConnID, fd, subReactors[rand].get());
        conn->setConnectCB(onConnectCB);
        conn->setMessageCB(onMessageCB);
        conn->setCloseCB([this](ConnectionTCP::CallbackParam c) { this->handleClose(c); });
        connectionMap[fd] = std::move(conn);
        connectionMap[fd]->establishConnection();
        nextConnID = nextConnID == 1001 ? 1 : nextConnID + 1;
    }

    void ServerTCP::handleClose(ConnectionTCP::CallbackParam conn) {
        println("CurrentThread: {},  TcpServer::HandleClose", currentThread::getTid());
        mainReactor->runOneFunc([this, conn]() { this->handleCloseInLoop(conn); });
    }

    void ServerTCP::handleCloseInLoop(ConnectionTCP::CallbackParam conn) {
        println(
            "CurrentThread: {} TcpServer::HandleCloseInLoop - Remove connection id: {} and fd {}",
            currentThread::getTid(), conn->getID(), conn->getFD()
        );
        const auto it = connectionMap.find(conn->getFD());
        assert(it != connectionMap.end());
        connectionMap.erase(it);
        EventLoop *loop = conn->getLoop();
        loop->queueFunc([conn]() { conn->destructConnection(); });
    }

    void ServerTCP::setConnectCB(ConnectionTCP::CallBackType fn) { onConnectCB = std::move(fn); }
    void ServerTCP::setMessageCB(ConnectionTCP::CallBackType fn) { onMessageCB = std::move(fn); }
}
