#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

import Config;
import Utils;
import Basekit;
import <string_view>;

using namespace std;


void input(int socketFD) {
    constexpr int bufSize = config::BUF_SIZE;
    while (true) {
        char bufData[bufSize + 1];
        char *buf = bufData;
        const auto readBytes = read(STDIN_FILENO, buf, bufSize);
        // 减1消去换行
        const auto writeBytes = write(socketFD, buf, readBytes - 1);
        utils::errIf(writeBytes == -1 || writeBytes != readBytes - 1, "write failed");

        bzero(buf, bufSize);
        const auto replyBytes = read(socketFD, buf, bufSize);
        buf[replyBytes] = '\0';
        if (replyBytes > 0) {
            println("read from server {} bytes: {}", replyBytes, buf);
        } else if (replyBytes == 0) {
            close(socketFD);
            println("server fd {} disconnected", socketFD);
        } else {
            close(socketFD);
            utils::errIf(true, "error occurred when  reading from server ");
        }
    }
}

int main() {
    using namespace basekit;
    const Socket sock{};
    const auto clientFD = sock.getFd();
    const InetAddress addr{config::ADDRESS, config::PORT};
    utils::errIf(
        connect(clientFD, addr.getReinter(), addr.getLen()),
        "connect failed"
    );
    input(clientFD);
}

// int main_prev() {
//     const int socketFD = socket(AF_INET, SOCK_STREAM, 0);
//     auto sockAddr = utils::buildSockAddrIn();
//     utils::errIf(
//         connect(socketFD, reinterpret_cast<sockaddr *>(&sockAddr), sizeof(sockAddr)),
//         "connect failed"
//     );
//
//     constexpr int bufSize = config::BUF_SIZE;
//     while (true) {
//         char bufData[bufSize + 1];
//         char *buf = bufData;
//         const auto readBytes = read(STDIN_FILENO, buf, bufSize);
//         // 减1消去换行
//         const auto writeBytes = write(socketFD, buf, readBytes - 1);
//         utils::errIf(writeBytes == -1 || writeBytes != readBytes - 1, "write failed");
//
//         bzero(buf, bufSize);
//         const auto replyBytes = read(socketFD, buf, bufSize);
//         buf[replyBytes] = '\0';
//         if (replyBytes > 0) {
//             println("read from server {} bytes: {}", replyBytes, buf);
//         } else if (replyBytes == 0) {
//             println("server fd {} disconnected", socketFD);
//             close(socketFD);
//         } else {
//             close(socketFD);
//             utils::errIf(true, "error occurred when  reading from server ");
//         }
//     }
// }
