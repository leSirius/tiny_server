module;
#include <unistd.h>

export module basekit:Connection;
import <cerrno>;
import <functional>;
import config;
import :socket;
import :buffer;
import :epollLoopChannel;

using namespace std;

namespace basekit {
    export class Connection {
    public:
        Connection(EventLoop *_loop, Socket *_sock);

        ~Connection();

        void echo(int sockFD);

        void setDeleteCallback(function<void(Socket *)> cb);

    private:
        EventLoop *loop;
        Socket *sock;
        string inBuffer{};
        Buffer readBuffer{};
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

    void Connection::echo(int sockFD) {
        constexpr auto READ_BUFFER = config::BUF_SIZE;
        char buf[READ_BUFFER]{};
        while (true) {
            const auto readBytes = read(sockFD, buf, READ_BUFFER);
            if (readBytes > 0) {
                readBuffer.append(buf);
            } else if (readBytes == 0) {
                deleteConnectionCB(sock);
                println("EOF, fd {} disconnected", sockFD);
                break;
            } else if (readBytes == -1) {
                if (errno == EINTR) {
                    println("interrupted, continue");
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    println("received from {}: {}", sockFD, readBuffer.c_str());
                    utils::errIf(write(sockFD, readBuffer.c_str(), readBuffer.size()) == -1, "server write error");
                    readBuffer.clear();
                    break;
                } else if (errno == ECONNRESET) {
                    readBuffer.clear();
                    deleteConnectionCB(sock);
                    println("client sent RST, fd {} disconnected", sockFD);
                    break;
                } else {
                    deleteConnectionCB(sock);
                    println("your server has reached no man's land", sockFD);
                    break;
                }
            }
            bzero(buf, sizeof(buf));
        }
    }

    void Connection::setDeleteCallback(function<void(Socket *)> cb) {
        deleteConnectionCB = std::move(cb);
    }
}
