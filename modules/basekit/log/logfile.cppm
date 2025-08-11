module;
#include <cassert>
export module basekit:logfile;
import <cstdint>;
import <cstdio>;
import <ctime>;
import <iostream>;
import <string>;
import <vector>;

import :timestamp;
import config;
import utils;

using namespace std;

export class LogFile {
public:
    explicit LogFile(const char *filepath = nullptr);

    ~LogFile();

    void writeTo(string_view data);

    void flush() const;

    [[nodiscard]] uint64_t getWrittenBytes() const;

private:
    FILE *fp;
    uint64_t writtenBytes{};
    time_t lastWriteTime{};
    time_t lastFlushTime{};
};


LogFile::LogFile(const char *filepath)
    : fp(fopen(filepath, "a+")) {
    if (!fp) {
        const auto path = config::LogPath(basekit::Timestamp::getNow().toString());
        fp = fopen(path.c_str(), "a+");
    }
    utils::errIf(fp == nullptr, "failed to open file");
}

LogFile::~LogFile() {
    flush();
    if (!fp) { fclose(fp); }
}

void LogFile::writeTo(const string_view data) {
    if (data.empty()) { return; }
    size_t pos = 0;
    const auto len = data.size();
    while (pos != len) {
        pos += fwrite_unlocked(data.data() + pos, sizeof(char), len - pos, fp);
    }
    const time_t now = time(nullptr);
    lastWriteTime = now;
    writtenBytes += len;
    if (lastWriteTime - lastFlushTime > config::FlushInterval) {
        flush();
        lastFlushTime = now;
    }
}

void LogFile::flush() const { fflush(fp); }

uint64_t LogFile::getWrittenBytes() const { return writtenBytes; }


