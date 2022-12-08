#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Containers/String.hpp"

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

enum LogLevel
{
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 0,
	LOG_LEVEL_INFO = 1,
	LOG_LEVEL_DEBUG = 2,
	LOG_LEVEL_TRACE = 3
};

struct NH_API File;

class NH_API Logger
{
public:
	template<typename... Types>
	static void Fatal(const char* message, const Types&... args)
	{
		String str(message);
		str.Format(args...);
		PrintError(str, LOG_LEVEL_FATAL);
	}
	template<typename Type>
	static void Fatal(const Type& arg)
	{
		String str(arg);
		PrintError(str, LOG_LEVEL_FATAL);
	}
	template<typename... Types>
	static void Error(const char* message, const Types&... args)
	{
		String str(message);
		str.Format(args...);
		PrintError(str, LOG_LEVEL_ERROR);
	}
	template<typename Type>
	static void Error(const Type& arg)
	{
		String str(arg);
		PrintError(str, LOG_LEVEL_ERROR);
	}
	template<typename... Types>
	static void Warn(const char* message, const Types&... args)
	{
#if LOG_WARN_ENABLED
		String str(message);
		str.Format(args...);
		Print(str, LOG_LEVEL_WARN);
#endif
	}
	template<typename Type>
	static void Warn(const Type& arg)
	{
#if LOG_WARN_ENABLED
		String str(arg);
		Print(str, LOG_LEVEL_WARN);
#endif
	}
	template<typename... Types>
	static void Info(const char* message, const Types&... args)
	{
#if LOG_INFO_ENABLED
		String str(message);
		str.Format(args...);
		Print(str, LOG_LEVEL_INFO);
#endif
	}
	template<typename Type>
	static void Info(const Type& arg)
	{
#if LOG_INFO_ENABLED
		String str(arg);
		Print(str, LOG_LEVEL_INFO);
#endif
	}
	template<typename... Types>
	static void Debug(const char* message, const Types&... args)
	{
#if LOG_DEBUG_ENABLED
		String str(message);
		str.Format(args...);
		Print(str, LOG_LEVEL_DEBUG);
#endif
	}
	template<typename Type>
	static void Debug(const Type& arg)
	{
#if LOG_DEBUG_ENABLED
		String str(arg);
		Print(str, LOG_LEVEL_DEBUG);
#endif
	}
	template<typename... Types>
	static void Trace(const char* message, const Types&... args)
	{
#if LOG_TRACE_ENABLED
		String str(message);
		str.Format(args...);
		Print(str, LOG_LEVEL_TRACE);
#endif
	}
	template<typename Type>
	static void Trace(const Type& arg)
	{
#if LOG_TRACE_ENABLED
		String str(arg);
		Print(str, LOG_LEVEL_TRACE);
#endif
	}

private:
	static bool Initialize();
	static void Shutdown();
	static void Print(String& message, LogLevel level);
	static void PrintError(String& message, LogLevel level);

	static File log;
#if defined PLATFORM_WINDOWS
	static void* consoleHandle;
	static void* errorHandle;
#endif

	Logger() = delete;

	friend class Engine;
};
