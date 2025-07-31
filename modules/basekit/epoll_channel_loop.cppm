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

    export class EventLoop {
    public:
        EventLoop();

        ~EventLoop();

        void loop() const;

        void updateChannel(Channel *ch) const;

        // void addThread(function<void()> func);

    private:
        Epoll *ep{nullptr};
        bool quit{false};
    };

    class Channel {
    private:
        EventLoop *loop{};
        int fd;
        uint32_t events{};
        uint32_t ready{};
        bool inEpoll{false};
        function<void()> readCallback{};
        function<void()> writeCallback{};

    public:
        Channel(EventLoop *_loop, int _fd);

        ~Channel();

        void enableReading();

        void enableET();

        [[nodiscard]] int getFd() const;

        [[nodiscard]] uint32_t getEvents() const;

        [[nodiscard]] uint32_t getReady() const;

        [[nodiscard]] bool getInEpoll() const;

        void setInEpoll(bool _in);

        void setReady(uint32_t);

        void setReadCallback(function<void()> r_cb);

        void setWriteCallback(function<void()> w_cb);

        void handleEvent() const;

        // void setUseThreadPool(bool _use);
    };
}
