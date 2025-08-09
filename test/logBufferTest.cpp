#include "logMacro.h"

import <array>;
import <iostream>;

import log;


int main() {
    // LogStream os;
    // os << "hello world";
    // std::cout << os.getBuffer().getSV() << std::endl;
    // os.resetBuffer();
    //
    // os << 11;
    // std::cout << os.getBuffer().getSV() << std::endl;
    // os.resetBuffer();
    //
    // os << 0.1;
    // std::cout << os.getBuffer().getSV() << std::endl;
    // os.resetBuffer();
    //
    // constexpr float d = 0.1;
    // os << Fmt("formated: {}", d);
    // std::cout << os.getBuffer().getSV() << std::endl;
    // os.resetBuffer();
    //
    // std::array<char, 20> bu{};
    // int i = 10;
    // std::format_to_n(bu.data(), bu.size(), "{}", i);
    Logger::setLogLevel(Logger::LogLevel::DEBUG);
    LOG_DEBUG << "BUG";
    LOG_INFO << "information";
    LOG_WARN << "This is a debug message.";
    LOG_ERROR << "This is an error message.";
    LOG_FATAL << "This is a fatal message.";
    return 0;
}
