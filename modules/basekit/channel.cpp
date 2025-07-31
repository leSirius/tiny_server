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
    Channel::Channel(EventLoop *_loop, const int _fd) : loop(_loop), fd(_fd) {
    }

    Channel::~Channel() {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }

    // void Channel::setUseThreadPool(const bool _use) {
    //     useThreadPool = _use;
    // }

    void Channel::enableReading() {
        events |= EPOLLIN | EPOLLPRI;
        loop->updateChannel(this);
    }

    void Channel::enableET() {
        events |= EPOLLET;
        loop->updateChannel(this);
    }

    int Channel::getFd() const {
        return fd;
    }

    uint32_t Channel::getEvents() const {
        return events;
    }

    uint32_t Channel::getReady() const {
        return ready;
    }

    bool Channel::getInEpoll() const {
        return inEpoll;
    }

    void Channel::setInEpoll(const bool _in) {
        inEpoll = _in;
    }

    void Channel::setReady(const uint32_t _ev) {
        ready = _ev;
    }

    void Channel::setReadCallback(function<void()> r_cb) {
        readCallback = std::move(r_cb);
    }

    void Channel::setWriteCallback(function<void()> w_cb) {
        writeCallback = std::move(w_cb);
    }

    void Channel::handleEvent() const {
        if (ready & (EPOLLIN | EPOLLPRI)) {
            readCallback();
        }
        if (ready & (EPOLLOUT)) {
            writeCallback();
        }
    }
}


// void Channel::handleEvent() const {
//     if (ready & (EPOLLIN | EPOLLPRI)) {
//         if (useThreadPool) {
//             loop->addThread(readCallback);
//         } else {
//             readCallback();
//         }
//     }
//     if (ready & EPOLLOUT) {
//         if (useThreadPool) {
//             // mainReactor->addThread(readCallback);
//             utils::errIf(true, "write call back called");
//             loop->addThread(writeCallback);
//         } else {
//             // readCallback();
//             writeCallback();
//         }
//     }
// }
