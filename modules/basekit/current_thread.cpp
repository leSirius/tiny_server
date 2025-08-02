module;
#include <format>
#include <unistd.h>
#include <sys/syscall.h>

module basekit;

import  <string>;

using namespace std;

namespace basekit::currentThread {
    thread_local pid_t t_cachedTid = 0;
    thread_local string t_cachedTidString;

    void CacheTid() {
        t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
        t_cachedTidString = format("{:5} ", t_cachedTid);
    }
}

