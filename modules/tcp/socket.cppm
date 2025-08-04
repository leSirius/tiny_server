module;
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>

export module tcp:socket;
import <iostream>;
import <string_view>;
import  <utility>;
import utils;
import :inetAddress;

using namespace std;


namespace tcp {
    export class Socket {
    public:
        Socket();

        explicit Socket(int _fd);

        ~Socket();

        void bind(const InetAddress &addr) const;

        void connect(const InetAddress &addr) const;

        void listen() const;

        void setNonBlock() const;

        int accept(InetAddress &addr) const;

        [[nodiscard]] auto acceptInner() const;

        [[nodiscard]] int getFd() const;

    private:
        int fd{-1};
    };

    Socket::Socket() {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        utils::errIf(fd == -1, "socket create error");
        constexpr auto optVal = 1;
        utils::errIf(
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1,
            "setsockopt failed"
        );
    }

    Socket::Socket(const int _fd): fd(_fd) {
    }


    Socket::~Socket() {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }

    void Socket::bind(const InetAddress &addr) const {
        utils::errIf(
            ::bind(fd, addr.getReinter(), addr.getLen()) == -1,
            "socket bind error"
        );
        println("bound to address: {}:{}", addr.getAddress(), addr.getPort());
    }

    void Socket::connect(const InetAddress &addr) const {
        utils::errIf(
            ::connect(fd, addr.getReinter(), addr.getLen()) == -1,
            "connect error"
        );
        println("connected to address: {}:{}", addr.getAddress(), addr.getPort());
    }


    void Socket::listen() const {
        utils::errIf(
            ::listen(fd, SOMAXCONN) == -1,
            "socket listen error"
        );
        println("listening on fd: {}", fd);
    }

    void Socket::setNonBlock() const {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    }

    int Socket::accept(InetAddress &addr) const {
        const int client_fd = ::accept(fd, addr.getReinterCC(), addr.getLenPtr());
        utils::errIf(client_fd == -1, "socket accept error");
        return client_fd;
    }

    auto Socket::acceptInner() const {
        InetAddress addr{};
        const int client_fd = ::accept(fd, addr.getReinterCC(), addr.getLenPtr());
        utils::errIf(client_fd == -1, "socket accept error");
        return std::make_pair(addr, client_fd);
    }


    int Socket::getFd() const { return fd; }
}
