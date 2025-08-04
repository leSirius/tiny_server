module;
#include <cassert>
#include <unistd.h>
export module tcp :tcpServer;
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
import :eventloop_threadpool;
namespace tcp {
    export class ServerTCP {
    public:
        ServerTCP(string_view IP, int port, int thdNum);

        ~ServerTCP();

        void start() const;

        // explicit TCPServer(EventLoop *);

        void handleNewConnect(int fd);

        void handleClose(ConnectionTCP::CallbackParam conn);

        void handleCloseInLoop(ConnectionTCP::CallbackParam conn);

        void setConnectCB(ConnectionTCP::CallBackType fn);

        void setMessageCB(ConnectionTCP::CallBackType fn);

        void setThreadNum(int num);

    private:
        int nextConnID{1};
        int threadNums{static_cast<int>(config::CPU_CORES)};
        unique_ptr<Eventloop> mainReactor{nullptr};
        unique_ptr<EventloopThreadpool> subReactorPool{nullptr};
        unique_ptr<Acceptor> acceptor;
        ConnectionTCP::CallBackType onConnectCB;
        ConnectionTCP::CallBackType onMessageCB;
        unordered_map<int, shared_ptr<ConnectionTCP> > connectionMap{};
    };

    ServerTCP::ServerTCP(const string_view IP, const int port, const int thdNum = static_cast<int>(config::CPU_CORES))
        : threadNums(thdNum) {
        mainReactor = make_unique<Eventloop>();
        acceptor = make_unique<Acceptor>(mainReactor.get(), IP, port);
        acceptor->setNewConnectCB([this](const int fd) { this->handleNewConnect(fd); });
        subReactorPool = std::make_unique<EventloopThreadpool>(mainReactor.get(), threadNums);

        //
        // const auto cores = config::CPU_CORES;
        // threadPool = make_unique<ThreadPool>(cores);
        // for (size_t i = 0; i < cores; ++i) {
        //     auto sub_reactor = make_unique<Eventloop>();
        //     subReactors.push_back(std::move(sub_reactor));
        // }
    }

    ServerTCP::~ServerTCP() = default;

    void ServerTCP::start() const {
        subReactorPool->start();
        mainReactor->loop();
        // for (auto &reactor: subReactors) {
        //     threadPool->add([r = reactor.get()] { r->loop(); });
        // }
    }

    void ServerTCP::handleNewConnect(const int fd) {
        assert(fd != -1);
        Eventloop *subReactor = subReactorPool->nextLoop();
        auto conn = make_shared<ConnectionTCP>(nextConnID, fd, subReactor);
        conn->setConnectCB(onConnectCB);
        conn->setMessageCB(onMessageCB);
        conn->setCloseCB([this](ConnectionTCP::CallbackParam c) { this->handleClose(c); });
        connectionMap[fd] = std::move(conn);
        connectionMap[fd]->establishConnection();
        nextConnID = nextConnID == 1001 ? 1 : nextConnID + 1;

        // const auto rand = fd % subReactors.size();
        // auto conn = make_shared<ConnectionTCP>(nextConnID, fd, subReactors[rand].get());
        // conn->setConnectCB(onConnectCB);
        // conn->setMessageCB(onMessageCB);
        // conn->setCloseCB([this](ConnectionTCP::CallbackParam c) { this->handleClose(c); });
        // connectionMap[fd] = std::move(conn);
        // connectionMap[fd]->establishConnection();
        // nextConnID = nextConnID == 1001 ? 1 : nextConnID + 1;
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
        Eventloop *loop = conn->getLoop();
        loop->queueFunc([conn]() { conn->destructConnection(); });
    }

    void ServerTCP::setConnectCB(ConnectionTCP::CallBackType fn) { onConnectCB = std::move(fn); }
    void ServerTCP::setMessageCB(ConnectionTCP::CallBackType fn) { onMessageCB = std::move(fn); }
    void ServerTCP::setThreadNum(const int num) { threadNums = num; }
}
