module;
#include <cassert>
#include <cstdint>
#include <unistd.h>
#include <sys/timerfd.h>

#include "logMacro.h"

module basekit;

import <chrono>;
import <functional>;
import <iostream>;
import <memory>;
import <ranges>;
import <set>;
import <vector>;

import utils;
import :timestamp;
import :countTimer;

using namespace std;

namespace basekit {
    TimerQueue::TimerQueue(Eventloop *_loop): loop(_loop) {
        createTimerFD();
        channel = make_unique<Channel>(loop, timerFD);
        channel->setReadCallback([this] { this->handleRead(); });
        channel->enableRead();
    }

    TimerQueue::~TimerQueue() {
        loop->deleteChannel(channel.get());
        close(timerFD);
    }

    void TimerQueue::createTimerFD() {
        timerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        utils::errIf(timerFD < 0, "TimerQueue::CreateTimerfd error");
    }

    void TimerQueue::readTimerFd() const {
        uint64_t content;
        if (const ssize_t readBytes = read(timerFD, &content, sizeof(content)); readBytes != sizeof(content)) {
            LOG_ERROR << "TimerQueue::ReadTimerFd read error";
        }
    }

    void TimerQueue::handleRead() {
        readTimerFd();
        activeTimers.clear();
        auto maxPtr = std::shared_ptr<CountTimer>(
            reinterpret_cast<CountTimer *>(UINTPTR_MAX),
            [](CountTimer *) {
            }
        );
        const auto end = timers.lower_bound(Entry(Timestamp::getNow(), maxPtr));
        activeTimers.insert(activeTimers.end(), timers.begin(), end);
        timers.erase(timers.begin(), end);
        for (const auto &val: activeTimers | views::values) { val->run(); }
        resetTimers();
    }

    void TimerQueue::setTime(const int fd, const Timestamp::Duration &diff) {
        itimerspec new_{};
        itimerspec old_{};
        const auto sec = duration_cast<chrono::seconds>(diff);
        const auto uSec = duration_cast<chrono::microseconds>(diff - sec);
        new_.it_value.tv_sec = static_cast<time_t>(sec.count());
        new_.it_value.tv_nsec = static_cast<long>(uSec.count() * 1000);
        const int ret = timerfd_settime(fd, 0, &new_, &old_);
        assert(ret != -1);
        (void) ret;
    }

    void TimerQueue::resetTimerFd(const CountTimer *timer) const {
        auto diff = timer->getExpiration().getTimePoint() - Timestamp::getNow().getTimePoint();
        if (diff < chrono::microseconds{100}) { diff = chrono::microseconds{100}; }
        setTime(timerFD, diff);
    }

    void TimerQueue::resetTimers() {
        for (const auto &timer: activeTimers | views::values) {
            if (timer->isRepeat()) {
                timer->reStart(Timestamp::getNow());
                insertTimer(timer);
            }
        }
        activeTimers.clear();
        if (!timers.empty()) {
            resetTimerFd(timers.begin()->second.get());
        }
    }

    bool TimerQueue::insertTimer(shared_ptr<CountTimer> timer) {
        bool resetInstantly = false;
        if (timers.empty() || timer->getExpiration() < timers.begin()->first) {
            resetInstantly = true;
        }
        timers.emplace(timer->getExpiration(), std::move(timer));
        return resetInstantly;
    }
}
