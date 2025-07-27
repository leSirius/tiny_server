#include <netinet/in.h>
#include <sys/socket.h>

import <array>;
import <csignal>;
import <iostream>;
import <string_view>;

import Config;
import Utils;

using namespace std;


auto buildSocketInTCP() {
    const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    utils::errIf(socket_fd == -1, "socket creation failed");
    constexpr int optVal = 1;
    utils::errIf(
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1,
        "setsockopt failed"
    );

    auto socket_addr = utils::buildSockAddrIn();
    utils::errIf(
        bind(socket_fd, reinterpret_cast<sockaddr *>(&socket_addr), sizeof(socket_addr)),
        "socket binding failed "
    );
    utils::errIf(listen(socket_fd, SOMAXCONN) == -1, "listening failed");
    return socket_fd;
}


void t_select() {
    fd_set read_fds{}, write_fds{};
    int ready{}, fd{}, numRead{}, j{}, nfds{};
    char buf[10];
    timeval timeout{};
    timeval *pto = &timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    const array<string, 3> inputs{
        "0r",
        "1w",
        "2r",
    };
    for (const auto &input: inputs) {
        numRead = sscanf(input.c_str(), "%d%2[rw]", &fd, buf);
        utils::errIf(numRead != 2, "sscanf() failed");
        utils::errIf(fd >= FD_SETSIZE, "file descriptor exceeds limit");
        if (fd >= nfds) { nfds = fd + 1; }
        if (strchr(buf, 'r') != nullptr) { FD_SET(fd, &read_fds); }
        if (strchr(buf, 'w') != nullptr) { FD_SET(fd, &write_fds); }
    }

    ready = select(nfds, &read_fds, &write_fds, nullptr, pto);
    utils::errIf(ready == -1, "select failed");
    println("ready = {}", ready);
    for (fd = 0; fd < nfds; fd++) {
        println(
            "{}: {}{}",
            fd, FD_ISSET(fd, &read_fds) ? "r" : "",FD_ISSET(fd, &write_fds) ? "w" : ""
        );
    }
    println("timeout after select(): {}.{}", timeout.tv_sec, timeout.tv_usec / 1000);
}


static volatile sig_atomic_t got_io{};

static void sigioHandler(int sig) {
    got_io = 1;
}

int ttySetCbreak(int fd, termios *prevTermios) {
    termios t{};
    if (tcgetattr(fd, &t) == -1)
        return -1;
    if (prevTermios != nullptr)
        *prevTermios = t;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_lflag |= ISIG;
    t.c_iflag &= ~ICRNL;
    t.c_cc[VMIN] = 1; /* Character-at-a-time input */
    t.c_cc[VTIME] = 0; /* with blocking */
    if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
        return -1;
    return 0;
}

void sigIO() {
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigioHandler;
    utils::errIf(
        sigaction(SIGIO, &sa, nullptr) == -1,
        "sigaction failed"
    );
    utils::errIf(
        fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1,
        "fcntl setting stdio own failed"
    );
    const auto flags = fcntl(STDIN_FILENO, F_GETFL);
    utils::errIf(
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1,
        "fcntl setting stdio async failed"
    );
    termios orig_termios{};
    utils::errIf(
        ttySetCbreak(STDIN_FILENO, &orig_termios) == -1,
        "ttysetcbreak failed"
    );
    auto done = false;
    while (!done) {
        sleep(1);
        if (got_io == 1) {
            char ch{};
            while (read(STDIN_FILENO, &ch, 1) > 0 && !done) {
                println("read: {}", ch);
                done = ch == '#';
            }
            got_io = 0;
        } else {
            println("lazy");
        }
    }
}


void handleReadEvent(const int socket_fd) {
    char buf[READ_BUFFER]{};
    while (true) {
        const ssize_t bytes_read = read(socket_fd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", socket_fd, buf);
            write(socket_fd, buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {
            printf("continue reading");
        } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if (bytes_read == 0) {
            printf("EOF, client fd %d disconnected\n", socket_fd);
            close(socket_fd);
            break;
        }
    }
}

int main_class() {
    using namespace basekit;
    const Socket serv_sock{};
    const auto server_fd = serv_sock.getFd();
    const InetAddress addr{config::ADDRESS, config::PORT};
    serv_sock.bind(addr);
    serv_sock.listen();
    serv_sock.setNonBlock();
    const auto ep = new Epoll{};
    const auto servChannel = new Channel{ep, server_fd};
    servChannel->enableReading();
    while (true) {
        auto activeChannels = ep->poll();
        const size_t nfds = activeChannels.size();
        for (int i = 0; i < nfds; ++i) {
            const int chfd = activeChannels[i]->getFd();
            if (chfd == server_fd) {
                InetAddress client_addr{};
                const auto client_sock = new Socket(serv_sock.accept(client_addr));
                println(
                    "new client fd {}! IP: {} Port: {}",
                    client_sock->getFd(), addr.getAddress(), addr.getPort()
                );
                client_sock->setNonBlock();
                auto *clientChannel = new Channel(ep, client_sock->getFd());
                clientChannel->enableReading();
            } else if (activeChannels[i]->getRevents() & EPOLLIN) {
                handleReadEvent(activeChannels[i]->getFd());
            } else {
                println("something else happened");
            }
        }
    }
}


void setNonBlock(const int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main_save() {
    using namespace basekit;
    const Socket socket{};
    const auto server_fd = socket.getFd();
    const InetAddress addr{config::ADDRESS, config::PORT};
    socket.bind(addr);
    socket.listen();

    int epfd = epoll_create1(0);
    utils::errIf(epfd == -1, "epoll create error");

    epoll_event events[MAX_EVENTS]{}, ev{};
    ev.data.fd = server_fd;
    ev.events = EPOLLIN | EPOLLET;
    setNonBlock(server_fd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        utils::errIf(nfds == -1, "epoll wait error");
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                sockaddr_in client_addr{};
                socklen_t client_addr_len = sizeof(client_addr);
                const int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);
                utils::errIf(client_fd == -1, "socket accept error");
                println("new client fd {}! from {}:{}", client_fd, inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port));
                bzero(&ev, sizeof(ev));
                ev.data.fd = client_fd;
                ev.events = EPOLLIN | EPOLLET;
                setNonBlock(client_fd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
            } else if (events[i].events & EPOLLIN) {
                char buf[READ_BUFFER]{};
                while (true) {
                    const ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        println("{} bytes from client fd {}: {}", bytes_read, events[i].data.fd, buf);
                        write(events[i].data.fd, buf, bytes_read);
                    } else if (bytes_read == -1 && errno == EINTR) {
                        println("continue reading");
                    } else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                        println("finish reading once, errno: {}", errno);
                        break;
                    } else if (bytes_read == 0) {
                        println("EOF, client fd {} disconnected", events[i].data.fd);
                        close(events[i].data.fd);
                        break;
                    }
                }
            } else {
                println("something else happened");
            }
        }
    }
    close(server_fd);
    // while (true) {
    //     sockaddr_in client_addr{};
    //     socklen_t client_addr_len = sizeof(client_addr);
    //     int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);
    //     utils::errIf(client_fd == -1, "socket accept error");
    //     println("new client fd: {}, IP: {}, Port: {}",
    //             client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    //
    //     constexpr auto buf_size = config::BUF_SIZE;
    //     char buf_data[buf_size + 1]{};
    //     char *buf = buf_data;
    //     ssize_t read_bytes{};
    //     do {
    //         read_bytes = read(client_fd, buf, buf_size);
    //         if (read_bytes > 0) {
    //             buf[read_bytes] = '\0';
    //             println("read from client fd {}: {}", client_fd, buf);
    //             write(client_fd, buf, read_bytes);
    //         }
    //     } while (read_bytes > 0);
    //
    //     if (read_bytes == 0) {
    //         println("client fd {} disconnected", client_fd);
    //         close(client_fd);
    //     } else if (read_bytes == -1) {
    //         close(client_fd);
    //         close(server_fd);
    //         utils::errIf(true, "socket read error");
    //     }
    // }

    return 0;
}


