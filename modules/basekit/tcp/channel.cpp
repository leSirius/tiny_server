module;

#include <unistd.h>
#include <sys/epoll.h>

module basekit;
import <cstdint>;
import <functional>;
import <iostream>;
import <utility>;
import utils;

namespace basekit {
    Channel::Channel(Eventloop *_loop, const int _fd) : loop(_loop), fd(_fd) {
    }

    Channel::~Channel() = default;

    void Channel::enableET() {
        listenEvents |= EPOLLET;
        loop->updateChannel(this);
    }

    void Channel::enableRead() {
        listenEvents |= EPOLLIN | EPOLLPRI;
        loop->updateChannel(this);
    }

    void Channel::enableWrite() {
        listenEvents |= EPOLLOUT;
        loop->updateChannel(this);
    }

    int Channel::getFd() const { return fd; }

    uint32_t Channel::getListenEvents() const { return listenEvents; }

    uint32_t Channel::getReadyEvents() const { return readyEvents; }

    bool Channel::isInEpoll() const { return inEpoll; }

    void Channel::setInEpoll(const bool _in) { inEpoll = _in; }

    void Channel::setReadyEvents(const uint32_t _ev) { readyEvents = _ev; }

    void Channel::setReadCallback(function<void()> r_cb) { readCallback = std::move(r_cb); }

    void Channel::setWriteCallback(function<void()> w_cb) { writeCallback = std::move(w_cb); }

    void Channel::handleEvent() const {
        if (tied) {
            shared_ptr<void> guard = tiePtr.lock();
            handleEventWithGuard();
        } else { handleEventWithGuard(); }
    }

    void Channel::handleEventWithGuard() const {
        if (readyEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP) && readCallback) { readCallback(); }
        if (readyEvents & EPOLLOUT && writeCallback) { writeCallback(); }
    }

    void Channel::tie(const shared_ptr<void> &ptr) {
        tied = true;
        tiePtr = ptr;
    }
}

