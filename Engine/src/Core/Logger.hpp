#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Containers/String.hpp"

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

    template<typename... Types>
    static NH_API void Fatal(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_FATAL, str);
        str.Destroy();
    }
    template<typename... Types>
    static NH_API void Error(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_ERROR, str);
        str.Destroy();
    }
    template<typename... Types>
    static NH_API void Warn(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_WARN, str);
        str.Destroy();
    }
    template<typename... Types>
    static NH_API void Info(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_INFO, str);
        str.Destroy();
    }
    template<typename... Types>
    static NH_API void Debug(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_DEBUG, str);
        str.Destroy();
    }
    template<typename... Types>
    static NH_API void Trace(const char* message, Types... args)
    {
        String str(message);
        str.Format(args...);
        LogOutput(LOG_LEVEL_TRACE, str);
        str.Destroy();
    }

private:
    static NH_API void LogOutput(LogLevel level, String& message); //TODO: Don't copy

    static struct File log;
    static const String levelStrings[];

    Logger() = delete;
};
