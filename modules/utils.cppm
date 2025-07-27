module;
#include <arpa/inet.h>
#include <sys/un.h>

export module Utils;
import  <cstdlib>;
import  <iostream>;
import  <string_view>;

import Config;

using namespace std;

namespace utils {
    export void errIf(const bool cond, string_view msg) {
        if (cond) {
            println("\033[31m{}\033[0m", msg);
            perror("from errno");
            exit(-1);
        }
    }

    export auto buildSockAddrIn(const string_view address = config::ADDRESS, const int port = config::PORT) {
        sockaddr_in socket_addr{};
        socket_addr.sin_family = AF_INET;
        errIf(inet_pton(AF_INET, address.data(), &socket_addr.sin_addr) != 1, "failed to bind address");
        socket_addr.sin_port = htons(port);
        return socket_addr;
    }

    export auto buildSockAddrUn(const string_view path) {
        sockaddr_un socket_addr{};
        socket_addr.sun_family = AF_UNIX;
        strcpy(socket_addr.sun_path, path.data());
        return socket_addr;
    }
}
