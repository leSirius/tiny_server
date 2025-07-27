#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>

import <array>;
import <csignal>;
import <iostream>;
import <string_view>;
import <vector>;

import config;
import utils;
import basekit;
using namespace std;

int main() {
    using namespace basekit;
    EventLoop eventLoop{};
    Server server(&eventLoop);
    eventLoop.loop();
    return 0;
}
