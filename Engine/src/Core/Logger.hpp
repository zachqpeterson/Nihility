#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Containers/String.hpp"

enum LogLevel
{
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_TRACE = 5
};

#define LOG_WARN_ENABLED 1
#define LOG_TRACE_ENABLED 0
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1

#ifdef NH_RELEASE
#undef LOG_DEBUG_ENABLED
#undef LOG_TRACE_ENABLED
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

struct File;

class NH_API Logger
{
public:
	template<typename... Types>
	static void Fatal(const char* message, const Types&... args)
	{
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_FATAL, str);
		str.Destroy();
	}
	template<typename Type>
	static void Fatal(const Type& arg)
	{
		String str(arg);
		LogOutput(LOG_LEVEL_FATAL, str);
	}
	template<typename... Types>
	static void Error(const char* message, const Types&... args)
	{
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_ERROR, str);
	}
	template<typename Type>
	static void Error(const Type& arg)
	{
		String str(arg);
		LogOutput(LOG_LEVEL_ERROR, str);
	}
	template<typename... Types>
	static void Warn(const char* message, const Types&... args)
	{
#if LOG_WARN_ENABLED
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_WARN, str);
#endif
	}
	template<typename Type>
	static void Warn(const Type& arg)
	{
#if LOG_WARN_ENABLED
		String str(arg);
		LogOutput(LOG_LEVEL_WARN, str);
#endif
	}
	template<typename... Types>
	static void Info(const char* message, const Types&... args)
	{
#if LOG_INFO_ENABLED
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_INFO, str);
#endif
	}
	template<typename Type>
	static void Info(const Type& arg)
	{
#if LOG_INFO_ENABLED
		String str(arg);
		LogOutput(LOG_LEVEL_INFO, str);
#endif
	}
	template<typename... Types>
	static void Debug(const char* message, const Types&... args)
	{
#if LOG_DEBUG_ENABLED
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_DEBUG, str);
#endif
	}
	template<typename Type>
	static void Debug(const Type& arg)
	{
#if LOG_DEBUG_ENABLED
		String str(arg);
		LogOutput(LOG_LEVEL_DEBUG, str);
#endif
	}
	template<typename... Types>
	static void Trace(const char* message, const Types&... args)
	{
#if LOG_TRACE_ENABLED
		String str(message);
		str.Format(args...);
		LogOutput(LOG_LEVEL_TRACE, str);
#endif
	}
	template<typename Type>
	static void Trace(const Type& arg)
	{
#if LOG_TRACE_ENABLED
		String str(arg);
		LogOutput(LOG_LEVEL_TRACE, str);
#endif
	}

private:
	static void LogOutput(LogLevel level, String& message);
	static bool Initialize();
	static void Shutdown();

	static File log;

	Logger() = delete;

	friend class Engine;
};
