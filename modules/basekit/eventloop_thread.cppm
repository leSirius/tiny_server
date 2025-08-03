module;

export module basekit:eventloopThread;
import <thread>;

import :epollLoopChannel;

using namespace std;

namespace basekit {
    export class EventloopThread {
    public:
        EventloopThread();

        ~EventloopThread();

        Eventloop *startLoop();

    private:
        Eventloop *loop{nullptr};
        jthread thread;
        mutex mtx;
        condition_variable cv;

        void ThreadFunc();
    };

    EventloopThread::EventloopThread() = default;

    EventloopThread::~EventloopThread() = default;

    Eventloop *EventloopThread::startLoop() {
        thread = jthread([this]() { this->ThreadFunc(); });
        Eventloop *loopCopy{nullptr}; {
            unique_lock lock(mtx);
            while (loop == nullptr) { cv.wait(lock); }
            loopCopy = loop;
        }
        return loopCopy;
    }

    void EventloopThread::ThreadFunc() {
        // notice here
        // EventloopThread newLoop();
        {
            unique_lock lock{mtx};
            loop = new Eventloop();
            cv.notify_one();
        }
        loop->loop(); {
            unique_lock lock{mtx};
            //  loop = nullptr;
            delete loop;
        }
    }
}


