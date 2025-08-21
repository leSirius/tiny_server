#include <unistd.h>

import  <iostream>;
import <string_view>;
import <vector>;

import config;
import utils;
import basekit;
using namespace std;

vector<string> testCases = {
    "GET / HTTP/1.1\r\n"
    "Host: www.httptest.com\r\n"
    "Connection: close\r\n"
    "\r\n",
    "GET /hello HTTP/1.1\r\n"
    "Host: www.httptest.com\r\n"
    "Connection: close\r\n"
    "\r\n",
};

vector<string> testCases2 = {
    "GET /hel",
    "lo HTTP",
    "/1.1\r\n",
    "Host: www.htt",
    "ptest.com\r\n",
    "Connec",
    "tion: close\r\n",
    "\r\n",
};

void sendHttp(int socketFD) {
    using namespace basekit;
    Buffer recvBuffer;
    constexpr int bufSize = config::BUF_SIZE;
    for (const auto &testReq: testCases2) {
        const auto writeBytes = write(socketFD, testReq.c_str(), testReq.size());
        if (writeBytes != testReq.size()) {
            println("sent {} of {}", writeBytes, testReq.size());
        }
        sleep(1);
    }
    char buf[bufSize]{};
    while (true) {
        const auto readBytes = read(socketFD, buf, bufSize);
        if (readBytes > 0) {
            recvBuffer.append(buf);
        } else if (readBytes == 0) {
            println("server closed connection", socketFD);
            break;
        } else {
            utils::errIf(true, "client reached no man's land ");
        }
        bzero(buf, sizeof(buf));
    }
    println("{}", recvBuffer.retrieveAllAsString());
    // for (const auto &testReq: testCases) {
    //     const auto writeBytes = write(socketFD, testReq.c_str(), testReq.size());
    //     println("sent: {}", testReq.c_str());
    //     utils::errIf(writeBytes == -1, "client sending message error");
    //     char buf[bufSize]{};
    //     while (true) {
    //         if (const auto readBytes = read(socketFD, buf, bufSize); readBytes > 0) {
    //             recvBuffer.append(buf);
    //         } else if (readBytes == 0) {
    //             println("server closed connection", socketFD);
    //             break;
    //         } else {
    //             utils::errIf(true, "client reached no man's land ");
    //         }
    //         bzero(buf, sizeof(buf));
    //     }
    //     recvBuffer.retrieveAll();
    // }
}

int main() {
    using namespace basekit;
    const Socket sock{};
    sock.connect({config::ADDRESS, config::PORT});
    sendHttp(sock.getFd());
}
