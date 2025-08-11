module;
#include <cassert>

export module basekit:logger;

import <chrono>;
import <functional>;
import <iostream>;
import <ratio>;

import :timestamp;
import :currentThread;
import :logStream;
import :fmt;

using namespace std;


class TimeArray {
public:
    static constexpr size_t LEN = 30;
    array<char, LEN> &getDataRef() { return arr; }

    [[nodiscard]] size_t getWritten() const { return written; }

    void setWritten(const size_t w) {
        assert(written <= LEN);
        written = w;
    }

private:
    array<char, LEN> arr{};
    size_t written{};
};

thread_local TimeArray timeArray;
thread_local long long t_lastSec;

// 不持有数据，注意生命周期问题
// class SourceFile {
// public:
//     explicit SourceFile(string_view source);
//
//     string_view fileName;
// };

// inline LogStream &operator<<(LogStream &s, const SourceFile &v) {
//     s.append(v.fileName);
//     return s;
// }

void defaultOutput(const string_view msg) {
    std::cout.write(msg.data(), static_cast<long>(msg.size()));;
}

void defaultFlush() { fflush(stdout); }

class LoggerImpl;

export class Logger {
public:
    enum class LogLevel {
        DEBUG, INFO, WARN, ERROR, FATAL
    };

    using OutputFunc = function<void(string_view msg)>;

    using FlushFunc = function<void()>;

    static OutputFunc outputStatic;

    static FlushFunc flushStatic;

    static LogLevel levelStatic;

    Logger(LogLevel _level, string_view _file, int _line);

    ~Logger();

    [[nodiscard]] LogStream &getStream() const;

    static LogLevel globalLevel();

    static void setLogLevel(LogLevel level);

    static void setOutput(OutputFunc);

    static void setFlush(FlushFunc);

private:
    unique_ptr<LoggerImpl> impl;
};

class LoggerImpl {
public:
    typedef Logger::LogLevel LogLevel;

    LoggerImpl(LogLevel _level, string_view source, int _line);

    void formattedTime();

    void finish();

    LogStream &getStream();

    [[nodiscard]] LogLevel getLevel() const;

    [[nodiscard]] const char *getLogLevelStr() const; // 获取LogLevel的字符串

private:
    string_view sourceFile;
    //sourceFile SourceFile sourceFile;
    int line;
    LogLevel level;
    LogStream stream;
};

Logger::OutputFunc Logger::outputStatic = defaultOutput;

Logger::FlushFunc Logger::flushStatic = defaultFlush;

Logger::LogLevel Logger::levelStatic = LogLevel::INFO;


// SourceFile::SourceFile(const string_view source) {
//     if (const auto pos = source.find_last_of('/'); pos != std::string_view::npos) {
//         fileName = source.substr(pos + 1);
//     } else {
//         fileName = source;
//     }
// }

Logger::Logger(LogLevel _level, const string_view _file, int _line): impl(
    make_unique<LoggerImpl>(_level, _file, _line)) {
}

Logger::~Logger() {
    impl->finish();
    const auto &buf{getStream().getBuffer()};
    outputStatic(buf.getSV());
    if (impl->getLevel() == LogLevel::FATAL) {
        flushStatic();
        abort();
    }
}

LogStream &Logger::getStream() const { return impl->getStream(); }

void Logger::setLogLevel(const LogLevel level) { levelStatic = level; }

void Logger::setOutput(OutputFunc func) { outputStatic = std::move(func); }

void Logger::setFlush(FlushFunc func) { flushStatic = std::move(func); }

LoggerImpl::LoggerImpl(const LogLevel _level, string_view source, const int _line)
    : line(_line), level(_level) {
    if (const auto pos = source.find_last_of('/'); pos != std::string_view::npos) {
        sourceFile = source.substr(pos + 1);
    } else {
        sourceFile = source;
    }

    formattedTime();
    stream << "TID[" << basekit::currentThread::tidString() << "]  ";
    stream << getLogLevelStr();
}

void LoggerImpl::formattedTime() {
    using namespace basekit;
    const auto now = Timestamp::getNow();
    const auto microSecTotal = chrono::microseconds(now.getTimePoint().time_since_epoch()).count();
    const auto sec = microSecTotal / chrono::microseconds::period::den;
    const auto uSec = microSecTotal % chrono::microseconds::period::den;
    if (t_lastSec != sec) {
        const auto written = now.toString(timeArray.getDataRef());
        timeArray.setWritten(written);
        t_lastSec = sec;
    }
    const Fmt us(".{:06}  ", uSec);
    this->stream << string_view(timeArray.getDataRef().data(), timeArray.getWritten()) << us.getSV();
}

void LoggerImpl::finish() { stream << "  -  " << sourceFile << ":" << line << "\n"; }

LogStream &LoggerImpl::getStream() { return stream; }

LoggerImpl::LogLevel LoggerImpl::getLevel() const { return level; }

const char *LoggerImpl::getLogLevelStr() const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG: ";
        case LogLevel::INFO:
            return "INFO: ";
        case LogLevel::WARN:
            return "WARN: ";
        case LogLevel::ERROR:
            return "ERROR: ";
        case LogLevel::FATAL:
            return "FATAL: ";
    }
    return nullptr;
}

Logger::LogLevel Logger::globalLevel() { return levelStatic; }

export inline LogStream &logMessage(const Logger::LogLevel level, const char *file, const int line) {
    return Logger(level, file, line).getStream();
}


