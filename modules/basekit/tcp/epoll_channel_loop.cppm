module;
#include <sys/epoll.h>

export module basekit:epollLoopChannel;

import <cstdint>;
import <functional>;
import <memory>;
import  <mutex>;
import <vector>;

import :timerQueue;

import :threadpool;

using namespace std;

namespace basekit {
    export class Channel;

    export class Epoll {
    private:
        int epfd;
        vector<epoll_event> eventVec;

    public:
        Epoll();

        ~Epoll();

        void updateChannel(Channel *) const;

        vector<Channel *> poll(int timeout = -1);

        void deleteChannel(Channel *channel) const;
    };

    export class Eventloop {
    public:
        Eventloop();

        ~Eventloop();

        void loop();

        void updateChannel(Channel *ch) const;

        void deleteChannel(Channel *ch) const;

        void doToDoList();

        void queueFunc(function<void()> func);

        [[nodiscard]] bool isInLoopThread() const;

        void handleRead() const;

        void runOneFunc(const function<void()> &func);

        void runAt(Timestamp timestamp, std::function<void()> const &cb) const;

        template<typename Rep, typename Period>
        void runAfter(chrono::duration<Rep, Period> dur, const std::function<void()> &cb) const;

        // void runAfter(int waitMicroSec, std::function<void()> const &cb) const;
        template<typename Rep, typename Period>
        void runEvery(chrono::duration<Rep, Period> dur, const std::function<void()> &cb) const;

    private
    :
        unique_ptr<Epoll> poller{};
        bool quit{false};
        vector<function<void()> > toDoList;
        mutex mtx;
        int wakeUpfD;
        unique_ptr<Channel> wakeUpChannel;
        bool callingFunc{false};
        pid_t threadID{0};
        unique_ptr<TimerQueue> timerQueue;
    };

    class Channel {
    public:
        Channel(Eventloop *_loop, int _fd);

        ~Channel();

        void handleEvent() const;

        void handleEventWithGuard() const;

        void enableET();

        void enableRead();

        void enableWrite();

        [[nodiscard]] int getFd() const;

        [[nodiscard]] uint32_t getListenEvents() const;

        [[nodiscard]] uint32_t getReadyEvents() const;

        [[nodiscard]] bool isInEpoll() const;

        void setInEpoll(bool _in);

        void setReadyEvents(uint32_t);

        void setReadCallback(function<void()> r_cb);

        void setWriteCallback(function<void()> w_cb);

        void tie(const std::shared_ptr<void> &ptr);

    private:
        Eventloop *loop{};
        int fd;
        uint32_t listenEvents{};
        uint32_t readyEvents{};
        bool inEpoll{false};
        function<void()> readCallback;
        function<void()> writeCallback;
        bool tied{false};
        std::weak_ptr<void> tiePtr;
    };

    void Eventloop::runAt(const Timestamp timestamp, function<void()> const &cb) const {
        timerQueue->addTimer(timestamp, cb);
    }

    template<typename Rep, typename Period>
    void Eventloop::runAfter(chrono::duration<Rep, Period> dur, const function<void()> &cb) const {
        timerQueue->addTimer(Timestamp::getAfter(dur), cb);
    }

    template<typename Rep, typename Period>
    void Eventloop::runEvery(chrono::duration<Rep, Period> dur, const function<void()> &cb) const {
        timerQueue->addTimer(Timestamp::getAfter(dur), cb, dur);
    }
}
