module;

export module basekit:threadpool;
import  <functional>;
import <mutex>;
import  <queue>;
import <thread>;
import  <vector>;

import config;
#include <print>


using namespace std;

namespace basekit {
    export class ThreadPool {
    public:
        explicit ThreadPool(unsigned int size = config::CPU_CORES);

        ~ThreadPool();

        void add(std::function<void()>);

    private:
        vector<thread> threads;
        queue<function<void()> > tasks;
        mutex tasks_mtx;
        condition_variable cv;
        bool stop{false};
    };

    ThreadPool::ThreadPool(const unsigned int size) {
        threads.reserve(size);
        for (int i = 0; i < size; ++i) {
            threads.emplace_back([this, i]() {
                while (true) {
                    function<void()> task; {
                        unique_lock lock(tasks_mtx);
                        cv.wait(lock, [this]() {
                            return stop || !tasks.empty();
                        });
                        if (stop && tasks.empty()) return;
                        task = tasks.front();
                        tasks.pop();
                    }
                    task();
                    println("from thread {}", i);
                }
            });
        }
    }

    ThreadPool::~ThreadPool() { {
            unique_lock lock(tasks_mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto &thread: threads) {
            if (thread.joinable()) { thread.join(); }
        }
    }

    void ThreadPool::add(function<void()> func) { {
            unique_lock lock(tasks_mtx);
            if (stop) { throw std::runtime_error("ThreadPool already stop, can't add task any more"); }
            tasks.emplace(std::move(func));
        }
        cv.notify_one();
    }
}
