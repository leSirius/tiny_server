#include <memory>

module;

export module basekit:asynclog;
import <barrier>;
import <latch>;
import <memory>;
import <mutex>;
import <thread>;
import <vector>;

import config;
import :timestamp;
import :fixedBuffer;
import :logfile;

using namespace std;

export class AsyncLog {
public:
    typedef FixedBuffer<> Buffer;

    explicit AsyncLog(const char *_filepath = nullptr);

    ~AsyncLog();

    void stop();

    void start();

    void append(string_view);

    static void flush();

    void threadFunc();

private:
    bool running;
    const char *filepath;
    std::mutex mutex;
    condition_variable cv;
    latch latcher;
    jthread jThread;
    unique_ptr<Buffer> current;
    unique_ptr<Buffer> next;
    vector<unique_ptr<Buffer> > buffers;
};

AsyncLog::AsyncLog(const char *_filepath)
    : running(false),
      filepath(_filepath),
      latcher(1) {
    current = make_unique<Buffer>();
    next = make_unique<Buffer>();
}

AsyncLog::~AsyncLog() { if (running) { stop(); } }

void AsyncLog::start() {
    running = true;
    jThread = jthread([this]() { this->threadFunc(); });
    latcher.wait();
}

void AsyncLog::stop() {
    running = false;
    cv.notify_one();
}

void AsyncLog::flush() { fflush(stdout); }

// 前端线程
void AsyncLog::append(const string_view data) {
    const auto len = data.size();
    unique_lock lock(mutex);
    if (current->leftSpace() >= len) {
        current->append(data);
    } else {
        buffers.push_back(std::move(current));
        if (next) { current = std::move(next); } else {
            current = std::make_unique<Buffer>();
        }
        current->append(data);
        cv.notify_one();
    }
}

// 后端线程
void AsyncLog::threadFunc() {
    latcher.count_down();
    auto newCurrent = make_unique<Buffer>();
    auto newNext = make_unique<Buffer>();
    auto logfile = make_unique<LogFile>();
    vector<unique_ptr<Buffer> > activeBuffers;
    while (running) {
        {
            unique_lock lock(mutex);
            if (buffers.empty()) {
                cv.wait_until(
                    lock,
                    basekit::Timestamp::getAfter(chrono::seconds{config::BufferWriteInterval}).getTimePoint()
                );
            }
            buffers.push_back(std::move(current));
            activeBuffers.swap(buffers);
            current = std::move(newCurrent);
            if (!next) { next = std::move(newNext); }
        }
        for (const auto &buf: activeBuffers) {
            const auto sv = buf->getSV();
            logfile->writeTo(string(sv));
        }
        if (logfile->getWrittenBytes() >= config::FileMaximumSize) {
            logfile = std::make_unique<LogFile>(filepath);
        }
        if (activeBuffers.size() > 2) { activeBuffers.resize(2); }
        if (!newCurrent) {
            newCurrent = std::move(activeBuffers.back());
            activeBuffers.pop_back();
            newCurrent->reset();
        }
        if (!newNext) {
            newNext = std::move(activeBuffers.back());
            activeBuffers.pop_back();
            newNext->reset();
        }
        activeBuffers.clear();
    }
}
