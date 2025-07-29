module;

export module basekit:threadpool;
import <functional>;
import <future>;
import <iostream>;
import <mutex>;
import <queue>;
import <thread>;
import <vector>;

import config;


using namespace std;

namespace basekit {
    export class ThreadPool {
    public:
        explicit ThreadPool(unsigned int size = config::CPU_CORES);

        ~ThreadPool();

        template<class F, class... Args>
        future<invoke_result_t<F, Args...> > add(F &&f, Args &&... args);

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

    template<typename F, typename... Args>
    future<invoke_result_t<F, Args...> > ThreadPool::add(F &&f, Args &&... args) {
        using ReturnType = invoke_result_t<F, Args...>;
        auto task = make_shared<packaged_task<ReturnType()> >(
            [f = std::forward<F>(f), args = make_tuple(forward<Args>(args)...)]() mutable {
                return apply(std::move(f), std::move(args));
            }
        );
        future<ReturnType> res = task->get_future(); {
            unique_lock lock(tasks_mtx);
            if (stop) { throw std::runtime_error("enqueue on stopped ThreadPool"); }
            tasks.emplace([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }
}


// void ThreadPool::add(function<void()> func) { {
//         unique_lock lock(tasks_mtx);
//         if (stop) { throw std::runtime_error("ThreadPool already stop, can't add task any more"); }
//         tasks.emplace(std::move(func));
//     }
//     cv.notify_one();
// }
