#include <unistd.h>

import  <iostream>;
import <string_view>;

import config;
import utils;
import basekit;
using namespace std;


void input(int socketFD) {
    using namespace basekit;
    Buffer sendBuffer, recvBuffer;
    constexpr int bufSize = config::BUF_SIZE;
    while (true) {
        sendBuffer.getLine();
        const auto writeBytes = write(socketFD, sendBuffer.c_str(), sendBuffer.size());
        utils::errIf(writeBytes == -1, "client sending message error");
        ssize_t alreadyRead = 0;
        char buf[bufSize]{};
        while (true) {
            const auto readBytes = read(socketFD, buf, bufSize);
            if (readBytes > 0) {
                recvBuffer.append(buf);
                alreadyRead += readBytes;
            } else if (readBytes == 0) {
                println("server closed connection", socketFD);
                return;
            } else {
                utils::errIf(true, "client reached no man's land ");
            }
            if (alreadyRead >= writeBytes) {
                println("message from server: {}", recvBuffer.c_str());
                break;
            }
            bzero(buf, sizeof(buf));
        }
        recvBuffer.clear();
    }
}

int main() {
    using namespace basekit;
    const Socket sock{};
    sock.connect({config::ADDRESS, config::PORT});
    input(sock.getFd());
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
