module;
#include  <cassert>


export module basekit:eventloop_threadpool;

import :eventloopThread;
import :epollLoopChannel;

namespace basekit {
    export class EventloopThreadpool {
    public:
        explicit EventloopThreadpool(Eventloop *_loop, int thdNum);

        ~EventloopThreadpool();

        void setThreadNums(int _threadNums);

        void start();

        Eventloop *nextLoop();

    private:
        Eventloop *mainReactor;
        vector<unique_ptr<EventloopThread> > threads;
        vector<Eventloop *> loops;
        int threadNums;
        int indNext{0};
    };

    EventloopThreadpool::EventloopThreadpool(Eventloop *_loop, const int thdNum)
        : mainReactor(_loop), threadNums(thdNum) {
    }

    EventloopThreadpool::~EventloopThreadpool() = default;

    void EventloopThreadpool::setThreadNums(const int _threadNums) { threadNums = _threadNums; }

    void EventloopThreadpool::start() {
        threads.reserve(threadNums);
        for (int i = 0; i < threadNums; ++i) {
            threads.emplace_back(make_unique<EventloopThread>());
            loops.emplace_back(threads.back()->startLoop());
        }
    }

    Eventloop *EventloopThreadpool::nextLoop() {
        Eventloop *ret = mainReactor;
        if (!loops.empty()) {
            ret = loops[indNext];
            indNext = indNext == static_cast<int>(loops.size()) - 1 ? 0 : indNext++;
        }
        return ret;
    }
}
