module;
#include <unistd.h>
#include <sys/epoll.h>

module basekit;
import <functional>;
import <iostream>;
import <string_view>;
import <vector>;
import utils;
import config;

using namespace std;

namespace basekit {
    Epoll::Epoll() {
        epfd = epoll_create1(0);
        utils::errIf(
            epfd == -1,
            "epoll_create1() failed"
        );
        eventVec.reserve(config::MAX_EVENTS);
    }

    Epoll::~Epoll() {
        if (epfd != -1) { close(epfd); }
        epfd = -1;
    }

    vector<Channel *> Epoll::poll(const int timeout) {
        vector<Channel *> activeChannels{};
        const int numFds = epoll_wait(epfd, eventVec.data(), config::MAX_EVENTS, timeout);
        utils::errIf(numFds == -1, "epoll wait error");
        activeChannels.reserve(numFds);
        for (int i = 0; i < numFds; ++i) {
            auto *ch = static_cast<Channel *>(eventVec[i].data.ptr);
            ch->setReady(eventVec[i].events);
            activeChannels.push_back(ch);
        }
        return activeChannels;
    }

    void Epoll::updateChannel(Channel *channel) const {
        const int fd = channel->getFd();
        epoll_event ev{};
        ev.data.ptr = channel;
        ev.events = channel->getEvents();
        if (!channel->getInEpoll()) {
            utils::errIf(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
            channel->setInEpoll(true);
        } else {
            utils::errIf(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
        }
    }

    void Epoll::deleteChannel(Channel *channel) const {
        const int fd = channel->getFd();
        utils::errIf(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1, "epoll delete error");
        channel->setInEpoll(false);
    }
}
