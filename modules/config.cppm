module;
#include <cassert>
#include <filesystem>
#include <iostream>
#include <unistd.h>
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

    export constexpr int BufferWriteInterval = 3;
    export constexpr int FlushInterval = 3;
    export constexpr int64_t FileMaximumSize = 1024 * 1024 * 1024;


    static string exePath{};

    export const string &getExeDirPath() {
        if (exePath.empty()) {
            char path[PATH_MAX];
            const auto count = readlink("/proc/self/exe", path, PATH_MAX);
            assert(count != -1);
            const filesystem::path fullPath(string(path, count));
            exePath = fullPath.parent_path().string();
        }
        return exePath;
    }

    export string LogPath(string name) {
        const auto path{getExeDirPath() + "/LogFiles/"};
        try {
            if (!std::filesystem::exists(path)) {
                if (std::filesystem::create_directories(path)) {
                } else {
                    println("{}", "failed to create directory");
                }
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return path + "LogFile_" + std::move(name) + ".log";
    }
}
