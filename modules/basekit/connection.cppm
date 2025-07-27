module;
#include <unistd.h>
export module Basekit:Connection;
import  <cerrno>;
import <functional>;
import Config;
import :Socket;
import :EpollLoopChannel;

using namespace std;

namespace basekit {
    export class Connection {
    public:
        Connection(EventLoop *_loop, Socket *_sock);

        ~Connection();

        void echo(int sockFD) const;

        void setDeleteCallback(function<void(Socket *)> cb);

    private:
        EventLoop *loop;
        Socket *sock;
        Channel *channel;
        function<void(Socket *)> deleteConnectionCB;
    };

    Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr) {
        const auto fd = sock->getFd();
        channel = new Channel(loop, fd);
        channel->setCallback([fd, this]() { this->echo(fd); });
        channel->enableReading();
    }

    Connection::~Connection() {
        delete channel;
        delete sock;
    }

    void Connection::echo(int sockFD) const {
        constexpr auto READ_BUFFER = config::BUF_SIZE;
        char buf[READ_BUFFER]{};
        while (true) {
            const auto readBytes = read(sockFD, buf, READ_BUFFER);
            if (readBytes > 0) {
                println("Received from fd: {} ", sockFD);
                write(sockFD, buf, readBytes);
            } else if (readBytes == 0) {
                deleteConnectionCB(sock);
                println("EOF, fd {} disconnected", sockFD);
                break;
            } else if (readBytes == -1) {
                if (errno == EINTR) {
                    println("interrupted, continue");
                    continue; // 中断后重试
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 非阻塞模式下，没有更多数据可读这
                    break;
                } else if (errno == ECONNRESET) {
                    deleteConnectionCB(sock);
                    println("client sent RST, fd {} disconnected", sockFD);
                    break;
                } else {
                    deleteConnectionCB(sock);
                    println("your server has reached no man's land", sockFD);
                    break;
                }
            }
        }
    }

    void Connection::setDeleteCallback(function<void(Socket *)> cb) {
        deleteConnectionCB = std::move(cb);
    }
}
