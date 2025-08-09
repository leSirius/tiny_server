module;
#include <arpa/inet.h>
#include <sys/un.h>

export module utils;
import  <cstdlib>;
import  <iostream>;
import  <string_view>;

import config;

using namespace std;

namespace utils {
    export void errIf(const bool cond, string_view msg) {
        if (cond) {
            println("\033[31m{}\033[0m", msg);
            perror("from errno");
            exit(-1);
        }
    }
}
