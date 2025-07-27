module;
#include <sys/epoll.h>

export module basekit:epollLoopChannel;
import  <cstdint>;
import  <functional>;
import  <vector>;
import :threadpool;

using namespace std;

namespace basekit {
    export class Channel;

    export class Epoll {
    public:
        Epoll();

        ~Epoll();

        void updateChannel(Channel *) const;

        vector<Channel *> poll(int timeout = -1);

    private:
        int epfd;
        vector<epoll_event> eventVec;
    };

    export class EventLoop {
    public:
        EventLoop();

        ~EventLoop();

        void loop() const;

        void updateChannel(Channel *ch) const;

        void addThread(function<void()> func);

    private:
        Epoll *ep{nullptr};
        ThreadPool threadPool;
        bool quit{false};
    };

    class Channel {
    public:
        Channel(EventLoop *_loop, int _fd);

        ~Channel() = default;

        void enableReading();

        [[nodiscard]] int getFd() const;

        [[nodiscard]] uint32_t getEvents() const;

        [[nodiscard]] uint32_t getRevents() const;

        [[nodiscard]] bool getInEpoll() const;

        void putInEpoll();

        void setRevents(uint32_t);

        void setCallback(std::function<void()> _cb);

        void handleEvent() const;

    private:
        EventLoop *loop{};
        int fd;
        uint32_t events{};
        uint32_t revents{};
        bool inEpoll{false};
        function<void()> callback{};
    };
}
