//
// Created by Calvin Tsui on 25-8-9.
//

#ifndef LOGMACRO_H
#define LOGMACRO_H

#define LOG_DEBUG   if (Logger::globalLevel() <= Logger::LogLevel::DEBUG) \
Logger(Logger::LogLevel::DEBUG, __FILE__, __LINE__ ).getStream()

#define LOG_INFO    if (Logger::globalLevel() <= Logger::LogLevel::INFO) \
Logger(Logger::LogLevel::INFO, __FILE__, __LINE__ ).getStream()

#define LOG_WARN    Logger(Logger::LogLevel::WARN, __FILE__, __LINE__ ).getStream()

#define LOG_ERROR   Logger(Logger::LogLevel::ERROR, __FILE__, __LINE__ ).getStream()

#define LOG_FATAL   Logger(Logger::LogLevel::FATAL, __FILE__, __LINE__ ).getStream()

#endif //LOGMACRO_H
