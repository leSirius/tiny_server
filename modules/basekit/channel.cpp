module;

#include <sys/epoll.h>

module basekit;
import  <cstdint>;
import <functional>;
import  <utility>;

namespace basekit {
    Channel::Channel(EventLoop *_loop, const int _fd) : loop(_loop), fd(_fd) {
    }

    void Channel::enableReading() {
        events = EPOLLIN | EPOLLET;
        loop->updateChannel(this);
    }

    int Channel::getFd() const {
        return fd;
    }

    uint32_t Channel::getEvents() const {
        return events;
    }

    uint32_t Channel::getRevents() const {
        return revents;
    }

    bool Channel::getInEpoll() const {
        return inEpoll;
    }

    void Channel::putInEpoll() {
        inEpoll = true;
    }

    void Channel::setRevents(const uint32_t _ev) {
        revents = _ev;
    }

    void Channel::setCallback(function<void()> _cb) {
        callback = std::move(_cb);
    }

    void Channel::handleEvent() const {
        loop->addThread(callback);
        // callback();
    }
}
