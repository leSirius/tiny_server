module;
#include <cassert>
#include <unistd.h>
#include <utility>
#include <sys/eventfd.h>

module basekit;
import <functional>;
import <memory>;
import <mutex>;
import <vector>;
using namespace std;

namespace basekit {
    Eventloop::Eventloop() {
        poller = std::make_unique<Epoll>();
        wakeUpfD = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        callingFunc = false;
        wakeUpChannel = make_unique<Channel>(this, wakeUpfD);
        wakeUpChannel->setReadCallback([this]() { this->handleRead(); });
        wakeUpChannel->enableRead();
    }

    Eventloop::~Eventloop() { close(wakeUpfD); }

    void Eventloop::loop() {
        threadID = currentThread::getTid();
        while (!quit) {
            for (vector chs{poller->poll()}; const auto &ch: chs) {
                ch->handleEvent();
            }
            doToDoList();
        }
    }

    void Eventloop::updateChannel(Channel *ch) const { poller->updateChannel(ch); }

    void Eventloop::deleteChannel(Channel *ch) const { poller->deleteChannel(ch); }

    void Eventloop::doToDoList() {
        callingFunc = true;
        vector<function<void()> > functors; {
            unique_lock lock(mtx);
            functors.swap(toDoList);
        }
        for (const auto &func: functors) { func(); }
        callingFunc = false;
    }

    void Eventloop::queueFunc(function<void()> func) { {
            unique_lock lock(mtx);
            toDoList.emplace_back(std::move(func));
        }
        if (!isInLoopThread() || callingFunc) {
            constexpr uint64_t writeOneByte = 1;
            const ssize_t writeByte = write(wakeUpfD, &writeOneByte, sizeof(writeOneByte));
            (void) writeByte;
            assert(writeByte == sizeof(writeOneByte));
        }
    }

    bool Eventloop::isInLoopThread() const { return currentThread::getTid() == threadID; }

    void Eventloop::handleRead() const {
        uint64_t readOneByte = 1;
        const ssize_t readSize = read(wakeUpfD, &readOneByte, sizeof(readOneByte));
        (void) readSize;
        assert(readSize == sizeof(readOneByte));
    }

    void Eventloop::runOneFunc(const function<void()> &func) {
        if (isInLoopThread()) { func(); } else {
            queueFunc(func);
        }
    }
}
