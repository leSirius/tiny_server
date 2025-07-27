export module Config;
import <cstddef>;
import  <string>;

using namespace std;

namespace config {
    export constexpr size_t BUF_SIZE{128};
    export constexpr string ADDRESS{"127.0.0.1"};
    export constexpr int PORT{8888};
    export constexpr int MAX_EVENTS{1024};
    export constexpr string SUN_PATH{"/tmp/random_addres"};
}
