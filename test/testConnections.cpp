#include <unistd.h>

import <cstring>;
import  <functional>;
import <iostream>;
import <thread>;
import <vector>;

import basekit;
import config;
import utils;

using namespace std;
using namespace basekit;

void oneClient(const int msgs, const int wait) {
    const Socket sock{};
    const InetAddress addr{config::ADDRESS, config::PORT};
    sock.connect(addr);
    const int sockFD = sock.getFd();

    Buffer sendBuffer{};
    Buffer readBuffer{};

    sleep(wait);
    int count = 0;
    while (count < msgs) {
        sendBuffer.append("I'm client!");
        utils::errIf(
            write(sockFD, sendBuffer.peekAllAsString().c_str(), sendBuffer.readableBytes()) == -1,
            "socket already disconnected, can't write any more!"
        );

        size_t already_read = 0;
        char buf[config::BUF_SIZE]{}; //这个buf大小无所谓
        while (true) {
            if (const auto read_bytes = read(sockFD, buf, sizeof(buf)); read_bytes > 0) {
                readBuffer.append(buf);
                already_read += read_bytes;
            } else if (read_bytes == 0) {
                printf("server disconnected!");
                exit(EXIT_SUCCESS);
            }
            if (already_read >= sendBuffer.readableBytes()) {
                println("count: {}, message from server: {}", count++, readBuffer.peekAllAsString());
                break;
            }
            bzero(&buf, sizeof(buf));
        }
        readBuffer.retrieveAll();
    }
}

int main(const int argc, char *argv[]) {
    int threads = 100;
    int msgs = 1;
    int wait = 0;
    int o;
    const auto optString = "t:m:w:";
    while ((o = getopt(argc, argv, optString)) != -1) {
        switch (o) {
            case 't':
                threads = stoi(optarg);
                break;
            case 'm':
                msgs = stoi(optarg);
                break;
            case 'w':
                wait = stoi(optarg);
                break;
            case '?':
                println("error optopt: {}", optopt);
                println("error opterr: {}", opterr);
                break;
            default: ;
                println("unknown option: {}", o);
        }
    }

    ThreadPool pool{static_cast<unsigned int>(threads)};

    auto func = [msgs, wait]() {
        oneClient(msgs, wait);
    };
    vector<jthread> jthreads{};
    for (int i = 0; i < threads; ++i) {
        // pool.add(func);
        jthreads.emplace_back(func);
    }
    return 0;
}
