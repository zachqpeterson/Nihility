#pragma once

#include "Defines.hpp"

enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 0

#ifdef NH_RELEASE
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

class Logger
{
public:
    static bool Initialize();
    static void Shutdown();

    static NH_API void LogOutput(LogLevel level, const char* message, ...);

private:
    Logger() = delete;

    static struct File log;
};

#define LOG_FATAL(message, ...) Logger::LogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) Logger::LogOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#if LOG_WARN_ENABLED
#define LOG_WARN(message, ...) Logger::LogOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
#define LOG_WARN(message, ...)
#endif
#if LOG_INFO_ENABLED
#define LOG_INFO(message, ...) Logger::LogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
#define LOG_INFO(message, ...)
#endif
#if LOG_DEBUG_ENABLED
#define LOG_DEBUG(message, ...) Logger::LogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif
#if LOG_TRACE_ENABLED
#define LOG_TRACE(message, ...) Logger::LogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define LOG_TRACE(message, ...)
#endif
