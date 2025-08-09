export module config;
import <cstddef>;
import  <string>;
import  <thread>;

using namespace std;

namespace config {
    export constexpr size_t BUF_SIZE{128};
    export constexpr size_t LOG_BUF_SIZE{4096};
    export constexpr string ADDRESS{"0.0.0.0"};
    export constexpr int PORT{8888};
    export constexpr int MAX_EVENTS{16384};
    export const unsigned int CPU_CORES{thread::hardware_concurrency()};
    export constexpr string SUN_PATH{"/tmp/random_address"};
    export constexpr chrono::seconds AUTO_CLOSE_TIME{5};
}
