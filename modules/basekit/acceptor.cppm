module;

export module Basekit:Acceptor;
import <functional>;

import Config;
import :EpollLoopChannel;
import :InetAddress;
import :Socket;

using namespace std;

namespace basekit {
    export class Acceptor {
    public:
        explicit Acceptor(EventLoop *_loop);

        // ~Acceptor();

        void acceptConnection() const;

        function<void(Socket *)> newConnectionCallback;

        void setNewConnectionCallback(const function<void(Socket *)> &);

    private:
        EventLoop *loop;
        unique_ptr<Socket> sock;
        unique_ptr<InetAddress> addr;
        unique_ptr<Channel> acceptChannel;
    };

    Acceptor::Acceptor(EventLoop *_loop) : loop(_loop) {
        addr = make_unique<InetAddress>(config::ADDRESS, config::PORT);
        sock = make_unique<Socket>();
        sock->bind(*addr);
        sock->listen();
        sock->setNonBlock();
        acceptChannel = make_unique<Channel>(loop, sock->getFd());
        acceptChannel->setCallback([this] { this->acceptConnection(); });
        acceptChannel->enableReading();
    }

    void Acceptor::acceptConnection() const {
        InetAddress clientAddr{};
        const auto clientSock = new Socket(sock->accept(clientAddr));
        clientSock->setNonBlock();
        newConnectionCallback(clientSock);
    }

    void Acceptor::setNewConnectionCallback(const function<void(Socket *)> &_cb) {
        newConnectionCallback = _cb;
    }
}
