module;

module Basekit;
import <vector>;


using namespace std;

namespace basekit {
    EventLoop::EventLoop() {
        ep = new Epoll();
    }

    EventLoop::~EventLoop() {
        delete ep;
    }

    void EventLoop::loop() const {
        while (!quit) {
            for (auto chs = ep->poll(); const auto ch: chs) {
                ch->handleEvent();
            }
        }
    }

    void EventLoop::updateChannel(Channel *ch) const {
        ep->updateChannel(ch);
    }
}
