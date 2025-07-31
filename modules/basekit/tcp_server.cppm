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


namespace basekit {
    export class ServerTCP {
    private:
        unique_ptr<EventLoop> mainReactor{nullptr};
        vector<unique_ptr<EventLoop> > subReactors;
        unique_ptr<Acceptor> acceptor;
        unordered_map<int, unique_ptr<ConnectionTCP> > connectionMap{};
        unique_ptr<ThreadPool> threadPool{};
        function<void(ConnectionTCP *)> onConnectCB{};
        function<void(ConnectionTCP *)> onMessageCB{};
        int nextConnID{1};

    public:
        ServerTCP(string_view IP, int port);

        ~ServerTCP();

        void start() const;

        // explicit TCPServer(EventLoop *);

        void handleNewConnection(int fd);

        void handleClose(int sockFD);

        void setConnectionCB(function<void(ConnectionTCP *)> fn);

        void setMessageCB(function<void(ConnectionTCP *)> fn);
    };

    ServerTCP::ServerTCP(string_view IP, int port) {
        mainReactor = make_unique<EventLoop>();
        acceptor = make_unique<Acceptor>(mainReactor.get(), IP, port);
        acceptor->setNewConnectCB([this](const int fd) { handleNewConnection(fd); });
        unsigned int cores = config::CPU_CORES;
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

    void ServerTCP::handleNewConnection(const int fd) {
        assert(fd != -1);
        const auto rand = fd % subReactors.size();
        auto conn = make_unique<ConnectionTCP>(nextConnID, fd, subReactors[rand].get());
        conn->setMessageCB(onMessageCB);
        conn->setCloseCB([this, fd]() { this->handleClose(fd); });
        connectionMap[fd] = std::move(conn);
        nextConnID = nextConnID == 1001 ? 1 : nextConnID + 1;
    }

    void ServerTCP::handleClose(const int sockFD) {
        if (connectionMap.contains(sockFD)) {
            connectionMap.erase(sockFD);
            close(sockFD);
        }
    }

    void ServerTCP::setConnectionCB(function<void(ConnectionTCP *)> fn) {
        onConnectCB = std::move(fn);
    }

    void ServerTCP::setMessageCB(function<void(ConnectionTCP *)> fn) {
        onMessageCB = std::move(fn);
    }
}
