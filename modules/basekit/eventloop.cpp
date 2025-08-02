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
    EventLoop::EventLoop() {
        poller = std::make_unique<Epoll>();
        wakeUpfD = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        callingFunc = false;
        wakeUpChannel = make_unique<Channel>(this, wakeUpfD);
        wakeUpChannel->setReadCallback([this]() { this->handleRead(); });
        wakeUpChannel->enableRead();
    }

    EventLoop::~EventLoop() {
        close(wakeUpfD);
    }

    void EventLoop::loop() {
        threadID = currentThread::getTid();
        while (!quit) {
            for (vector chs{poller->poll()}; const auto &ch: chs) {
                ch->handleEvent();
            }
            doToDoList();
        }
    }

    void EventLoop::updateChannel(Channel *ch) const { poller->updateChannel(ch); }

    void EventLoop::deleteChannel(Channel *ch) const { poller->deleteChannel(ch); }

    void EventLoop::doToDoList() {
        callingFunc = true;
        vector<function<void()> > functors; {
            unique_lock lock(mtx);
            functors.swap(toDoList);
        }
        for (const auto &func: functors) { func(); }
        callingFunc = false;
    }

    void EventLoop::queueFunc(function<void()> func) { {
            unique_lock lock(mtx);
            toDoList.emplace_back(std::move(func));
        }
        // 如果调用当前函数的并不是当前当前EventLoop对应的的线程，将其唤醒。主要用于关闭TcpConnection
        // 由于关闭连接是由对应`TcpConnection`所发起的，但是关闭连接的操作应该由main_reactor所进行(为了释放ConnectionMap的所持有的TcpConnection)
        if (!isInLoopThread() || callingFunc) {
            constexpr uint64_t writeOneByte = 1;
            const ssize_t writeByte = write(wakeUpfD, &writeOneByte, sizeof(writeOneByte));
            (void) writeByte;
            assert(writeByte == sizeof(writeOneByte));
        }
    }

    bool EventLoop::isInLoopThread() const { return currentThread::getTid() == threadID; }

    void EventLoop::handleRead() const {
        uint64_t readOneByte = 1;
        const ssize_t readSize = read(wakeUpfD, &readOneByte, sizeof(readOneByte));
        (void) readSize;
        assert(readSize == sizeof(readOneByte));
    }

    void EventLoop::runOneFunc(const function<void()> &func) {
        if (isInLoopThread()) { func(); } else {
            queueFunc(func);
        }
    }
}
