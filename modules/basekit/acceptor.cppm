module;

#include <cassert>
#include <arpa/inet.h>
#include <sys/socket.h>
export module basekit:acceptor;
import <functional>;

import config;
import :epollLoopChannel;
import :inetAddress;
import :socket;


using namespace std;

namespace basekit {
    export class Acceptor {
    public:
        Acceptor(Eventloop *_loop, string_view ip, int port);

        // ~Acceptor();

        void createSock();

        void bindAddr(string_view ip, int port) const;

        void listenBegin() const;


        void acceptConnection() const;


        void setNewConnectCB(function<void(int)> _cb);

    private:
        Eventloop *loop;
        int listenFD{-1};
        unique_ptr<Channel> acceptChannel{};
        function<void(int)> newConnectCB;
    };

    Acceptor::Acceptor(Eventloop *_loop, const string_view ip, const int port) : loop(_loop) {
        createSock();
        bindAddr(ip, port);
        listenBegin();
        acceptChannel = make_unique<Channel>(loop, listenFD);
        acceptChannel->setReadCallback([this] { this->acceptConnection(); });
        acceptChannel->enableRead();
    }

    void Acceptor::createSock() {
        assert(listenFD == -1);
        listenFD = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        utils::errIf(listenFD == -1, "Failed to create socket in acceptor");
        constexpr int optVal = 1;
        utils::errIf(
            setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0,
            "setting SO_REUSEADDR failed"
        );
    }

    void Acceptor::bindAddr(const string_view ip, const int port) const {
        const InetAddress addr{ip, port};
        utils::errIf(
            ::bind(listenFD, addr.getReinterCC(), addr.getLen()) == -1,
            "Failed to bind address to port in acceptor"
        );
    }

    void Acceptor::listenBegin() const {
        assert(listenFD != -1);
        utils::errIf(listen(listenFD, SOMAXCONN) == -1, "failed to listen on port in acceptor");
    }

    void Acceptor::acceptConnection() const {
        assert(listenFD != -1);
        InetAddress clientAddr{};
        const int clientFD = accept4(
            listenFD, clientAddr.getReinterCC(), clientAddr.getLenPtr(),SOCK_NONBLOCK | SOCK_CLOEXEC
        );
        utils::errIf(clientFD == -1, "Failed to accept connection in acceptor");
        println("New client: {}, {}:{}", clientFD, clientAddr.getAddress(), clientAddr.getPort());
        if (newConnectCB) { newConnectCB(clientFD); } else {
            utils::errIf(true, "missing connection call back in acceptor");
        }
    }

    void Acceptor::setNewConnectCB(function<void(int)> _cb) { newConnectCB = std::move(_cb); }
}
