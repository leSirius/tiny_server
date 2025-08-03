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
import :currentThread;

using namespace std;

namespace basekit {
    export class ConnectionTCP : public enable_shared_from_this<ConnectionTCP> {
    public:
        using CallbackParam = const shared_ptr<ConnectionTCP> &;
        using CallBackType = function<void(CallbackParam)>;

        enum class State { Invalid, Connected, Disconnected };

        ConnectionTCP(int id, int fd, Eventloop *_loop);

        ~ConnectionTCP();

        void establishConnection();

        void destructConnection();

        void setConnectCB(CallBackType cb);

        void setMessageCB(CallBackType cb);

        void setCloseCB(CallBackType cb);

        void setSendBuffer(string_view _buffer);

        [[nodiscard]] const Buffer *getSendBuffer() const;

        [[nodiscard]] const Buffer *getRecvBuffer() const;

        [[nodiscard]] string getRecvContent() const;

        void sendMsg(string_view msg);

        void handleMessage();

        void handleClose();

        [[nodiscard]] State getState() const;

        [[nodiscard]] Eventloop *getLoop() const;

        [[nodiscard]] int getFD() const;

        [[nodiscard]] int getID() const;

    private:
        int connFD;
        int connID;
        State state{State::Invalid};
        Eventloop *loop;
        Channel channel;
        Buffer recvBuffer{};
        Buffer sendBuffer{};
        CallBackType onConnectCB;
        CallBackType onMessageCB;
        CallBackType onCloseCB;

        void setState(State _state);

        void readToRBuf();

        void writeFromSBuf();

        void readNonBlock();

        void writeNonBlock();
    };

    ConnectionTCP::ConnectionTCP(const int id, const int fd, Eventloop *_loop)
        : connFD(fd), connID(id), loop(_loop), channel(Channel(loop, fd)) {
        if (loop != nullptr) {
            channel.enableET();
            channel.setReadCallback([this]() { this->handleMessage(); });
        } else {
            utils::errIf(true, "null loop ptr while building connection");
        }
        recvBuffer = Buffer();
        sendBuffer = Buffer();
    }

    ConnectionTCP::~ConnectionTCP() { close(connFD); }

    void ConnectionTCP::establishConnection() {
        setState(State::Connected);
        channel.tie(shared_from_this());
        channel.enableRead();
        if (onConnectCB) {
            onConnectCB(shared_from_this());
        }
    }

    void ConnectionTCP::destructConnection() {
        println("{},  ConnectionTCP::destructConnection", currentThread::getTid());
        loop->deleteChannel(&channel);
    }

    void ConnectionTCP::readToRBuf() {
        // 这里可能会有 Disconnected 问题!!! 待检查解决
        assert(state==State::Connected);
        utils::errIf(state != State::Connected, "state is " + to_string(static_cast<int>(state)));
        recvBuffer.clearBuf();
        readNonBlock();
    }

    void ConnectionTCP::writeFromSBuf() {
        assert(state == State::Connected);
        writeNonBlock();
        sendBuffer.clearBuf();
    }

    void ConnectionTCP::sendMsg(const string_view msg) {
        setSendBuffer(msg);
        writeFromSBuf();
    }

    void ConnectionTCP::handleMessage() {
        readToRBuf();
        if (onMessageCB) { onMessageCB(shared_from_this()); } else {
            utils::errIf(true, "null handleMessageCB in Connection");
        }
    }

    void ConnectionTCP::handleClose() {
        if (state != State::Disconnected) {
            setState(State::Disconnected);
            if (onCloseCB) { onCloseCB(shared_from_this()); } else {
                utils::errIf(true, "null handleCloseCB in Connection");
            }
        }
    }

    Eventloop *ConnectionTCP::getLoop() const { return loop; }

    int ConnectionTCP::getFD() const { return connFD; }

    int ConnectionTCP::getID() const { return connID; }

    void ConnectionTCP::setConnectCB(CallBackType cb) { onConnectCB = std::move(cb); }

    void ConnectionTCP::setMessageCB(CallBackType cb) { onMessageCB = std::move(cb); }

    void ConnectionTCP::setCloseCB(CallBackType cb) { onCloseCB = std::move(cb); }

    void ConnectionTCP::setSendBuffer(const string_view _buffer) { sendBuffer.setBuf(_buffer); }

    const Buffer *ConnectionTCP::getSendBuffer() const { return &sendBuffer; }

    const Buffer *ConnectionTCP::getRecvBuffer() const { return &recvBuffer; }

    string ConnectionTCP::getRecvContent() const { return getRecvBuffer()->c_str(); }

    ConnectionTCP::State ConnectionTCP::getState() const { return state; }

    void ConnectionTCP::setState(const State _state) { state = _state; }

    void ConnectionTCP::readNonBlock() {
        int sockFD = connFD;
        char buf[config::BUF_SIZE]{};
        while (true) {
            if (const auto bytesRead = read(sockFD, buf, sizeof(buf)); bytesRead > 0) {
                recvBuffer.append(buf);
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
