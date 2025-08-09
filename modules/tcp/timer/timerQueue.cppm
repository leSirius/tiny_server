module;

export module tcp:timerQueue;
import <functional>;
import <iostream>;
import <memory>;
import <ranges>;
import <set>;
import <vector>;

// import tcp;

import utils;
import :timestamp;
import :countTimer;

using namespace std;


namespace tcp {
    export class Eventloop;
    export class Channel;

    export class TimerQueue {
        typedef pair<Timestamp, shared_ptr<CountTimer> > Entry;

        struct EntryCompare {
            bool operator()(const Entry &a, const Entry &b) const {
                return a.first < b.first || (a.first == b.first && a.second.get() < b.second.get());
            }
        };

    public:
        explicit TimerQueue(Eventloop *_loop);

        ~TimerQueue();

        void createTimerFD();

        void readTimerFd() const;

        void handleRead();

        void resetTimerFd(const CountTimer *timer) const;

        void resetTimers();

        bool insertTimer(shared_ptr<CountTimer> timer);


        void addTimer(Timestamp timestamp, std::function<void()> const &cb);

        template<typename Rep, typename Period>
        void addTimer(
            Timestamp timestamp, std::function<void()> const &cb, chrono::duration<Rep, Period> interval
        );

    private:
        Eventloop *loop;
        int timerFD{-1};
        unique_ptr<Channel> channel;

        set<Entry, EntryCompare> timers;

        vector<Entry> activeTimers;

        static void setTime(int fd, const Timestamp::Duration &diff);
    };

    void TimerQueue::addTimer(Timestamp timestamp, std::function<void()> const &cb) {
        if (const auto timer = make_shared<CountTimer>(timestamp, cb);
            insertTimer(timer)) {
            resetTimerFd(timer.get());
        }
    }

    template<typename Rep, typename Period>
    void TimerQueue::addTimer(
        const Timestamp timestamp, std::function<void()> const &cb, const chrono::duration<Rep, Period> interval
    ) {
        if (const auto timer = make_shared<CountTimer>(timestamp, cb, interval);
            insertTimer(timer)) {
            resetTimerFd(timer.get());
        }
    }
}
