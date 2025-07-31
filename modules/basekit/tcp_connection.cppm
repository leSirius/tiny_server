module;
#include <cassert>
#include <unistd.h>

export module basekit:tcpConnection;
import <cerrno>;
import <functional>;

import config;
import :socket;
import :buffer;
import :epollLoopChannel;

using namespace std;

namespace basekit {
    export class ConnectionTCP {
    public:
        enum class State {
            Invalid,
            Connected,
            Disconnected,
        };

        ConnectionTCP(int id, int fd, EventLoop *_loop);

        ~ConnectionTCP();

        void setMessageCB(function<void(ConnectionTCP *)> callback);

        void setCloseCB(function<void()> const &callback);

        void setSendBuffer(string_view _buffer);

        [[nodiscard]] const Buffer *getSendBuffer() const;

        [[nodiscard]] const Buffer *getReadBuffer() const;

        void readToRBuf();

        void writeFromSBuf();

        void sendMsg(string_view msg);

        void handleMessage();

        void handleClose();

        [[nodiscard]] State getState() const;

        [[nodiscard]] EventLoop *getLoop() const;

        [[nodiscard]] int getFD() const;

        [[nodiscard]] int getID() const;

    private:
        int connFD{};
        int connID{};
        State state{State::Invalid};
        EventLoop *loop;
        Channel channel;
        Buffer readBuffer{};
        Buffer sendBuffer{};
        function<void(ConnectionTCP *)> onMessageCB;
        function<void()> onCloseCB;

        void setState(State _state);

        void readNonBlock();

        void writeNonBlock();
    };

    ConnectionTCP::ConnectionTCP(const int id, const int fd, EventLoop *_loop)
        : connFD(fd), connID(id), loop(_loop), channel(Channel(loop, fd)) {
        if (loop != nullptr) {
            channel.enableET();
            channel.setReadCallback([this]() { this->handleMessage(); });
            channel.enableReading();
        } else {
            utils::errIf(true, "null loop ptr while building connection");
        }
        readBuffer = Buffer();
        sendBuffer = Buffer();
    }

    ConnectionTCP::~ConnectionTCP() { close(connFD); }

    void ConnectionTCP::readToRBuf() {
        // assert(state==State::Connected);
        readBuffer.clearBuf();
        readNonBlock();
    }

    void ConnectionTCP::writeFromSBuf() {
        // assert(state == State::Connected);
        writeNonBlock();
        sendBuffer.clearBuf();
    }

    void ConnectionTCP::sendMsg(const string_view msg) {
        setSendBuffer(msg);
        writeFromSBuf();
    }

    void ConnectionTCP::handleMessage() {
        readToRBuf();
        if (onMessageCB) { onMessageCB(this); } else {
            utils::errIf(true, "null handleMessageCB in Connection");
        }
    }

    void ConnectionTCP::handleClose() {
        if (state != State::Disconnected) {
            setState(State::Disconnected);
            if (onCloseCB) { onCloseCB(); } else {
                utils::errIf(true, "null handleCloseCB in Connection");
            }
        }
    }

    EventLoop *ConnectionTCP::getLoop() const { return loop; }

    int ConnectionTCP::getFD() const { return connFD; }

    int ConnectionTCP::getID() const { return connID; }

    void ConnectionTCP::setMessageCB(function<void(ConnectionTCP *)> callback) {
        onMessageCB = std::move(callback);
    }

    void ConnectionTCP::setCloseCB(function<void()> const &callback) { onCloseCB = callback; }

    void ConnectionTCP::setSendBuffer(const string_view _buffer) { sendBuffer.setBuf(_buffer); }

    const Buffer *ConnectionTCP::getSendBuffer() const { return &sendBuffer; }

    const Buffer *ConnectionTCP::getReadBuffer() const { return &readBuffer; }

    ConnectionTCP::State ConnectionTCP::getState() const { return state; }

    void ConnectionTCP::setState(const State _state) { state = _state; }

    void ConnectionTCP::readNonBlock() {
        int sockFD = connFD;
        char buf[config::BUF_SIZE]{};
        while (true) {
            if (const auto bytesRead = read(sockFD, buf, sizeof(buf)); bytesRead > 0) {
                readBuffer.append(buf);
            } else if (bytesRead == 0) {
                println("read EOF, client fd {} disconnected", sockFD);
                handleClose();
                break;
            } else if (bytesRead == -1) {
                if (errno == EINTR) {
                    println("EINTR, continue reading");
                    // continue;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    println("Other error on client fd {}", sockFD);
                    handleClose();
                    break;
                }
            }
            bzero(buf, sizeof(buf));
        }
    }

    void ConnectionTCP::writeNonBlock() {
        const auto sockFD = connFD;
        const string buf{sendBuffer.c_str()};
        const auto dataSize = buf.size();
        auto dataLeft = dataSize;

        while (dataLeft > 0) {
            const auto bytesWrite = write(sockFD, buf.c_str() + dataSize - dataLeft, dataLeft);
            if (bytesWrite == -1) {
                if (errno == EINTR) { continue; } else if (errno == EAGAIN) { break; } else {
                    println("Other error on connection writing , fd: {}", sockFD);
                    handleClose();
                    break;
                }
            }
            dataLeft -= bytesWrite;
        }
    }
}
