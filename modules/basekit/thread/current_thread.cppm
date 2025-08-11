module;
#include <format>
#include <sys/types.h>
export module basekit:currentThread;
import <cstdio>;
import <string>;
using namespace std;

namespace basekit::currentThread {
    extern thread_local pid_t t_cachedTid;
    extern thread_local string t_cachedTidString;

    void CacheTid();

    export inline pid_t getTid() {
        if (t_cachedTid == 0) [[unlikely]] { CacheTid(); }
        return t_cachedTid;
    }

    export inline const string &tidString() {
        if (t_cachedTid == 0) [[unlikely]] { CacheTid(); }
        return t_cachedTidString;
    }
}
