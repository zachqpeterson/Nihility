#pragma once

#include "Defines.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef NH_DEBUG
#	define LOG_TRACE_ENABLED 1
#	define LOG_DEBUG_ENABLED 1
#else
#	define LOG_DEBUG_ENABLED 0
#	define LOG_TRACE_ENABLED 0
#endif

struct String;

class NH_API Logger
{
public:
	template<typename... Types> static void Fatal(const char* message, const Types&... args);
	template<typename... Types> static void Error(const char* message, const Types&... args);
	template<typename... Types> static void Warn(const char* message, const Types&... args);
	template<typename... Types> static void Debug(const char* message, const Types&... args);
	template<typename... Types> static void Info(const char* message, const Types&... args);
	template<typename... Types> static void Trace(const char* message, const Types&... args);
	template<typename Types> static void Fatal(const Types& arg);
	template<typename Types> static void Error(const Types& arg);
	template<typename Types> static void Warn(const Types& arg);
	template<typename Types> static void Debug(const Types& arg);
	template<typename Types> static void Info(const Types& arg);
	template<typename Types> static void Trace(const Types& arg);

private:
	static bool Initialize();
	static void Shutdown();

	static void Print(const String& message, U16 color);
	static void PrintError(const String& message, U16 color);

	static String fatalTag;
	static String errorTag;
	static String warnTag;
	static String debugTag;
	static String infoTag;
	static String traceTag;
	static String endLine;

#if defined PLATFORM_WINDOWS
	static void* consoleHandle;
	static void* errorHandle;
#endif

	STATIC_CLASS(Logger);
	friend class Engine;
};

template<typename... Types> inline void Logger::Fatal(const char* message, const Types&... args)
{
	String str(message, args...);
	str.Surround(fatalTag, endLine);
	PrintError(str, 64);
}

template<typename... Types> inline void Logger::Error(const char* message, const Types&... args)
{
	String str(message, args...);
	str.Surround(errorTag, endLine);
	PrintError(str, 4);
}

template<typename... Types> inline void Logger::Warn(const char* message, const Types&... args)
{
#if LOG_WARN_ENABLED
	String str(message, args...);
	str.Surround(warnTag, endLine);
	Print(str, 6);
#endif
}

template<typename... Types> inline void Logger::Info(const char* message, const Types&... args)
{
#if LOG_INFO_ENABLED
	String str(message, args...);
	str.Surround(infoTag, endLine);
	Print(str, 10);
#endif
}

template<typename... Types> inline void Logger::Debug(const char* message, const Types&... args)
{
#if LOG_DEBUG_ENABLED
	String str(message, args...);
	str.Surround(debugTag, endLine);
	Print(str, 11);
#endif
}

template<typename... Types> inline void Logger::Trace(const char* message, const Types&... args)
{
#if LOG_TRACE_ENABLED
	String str(message, args...);
	str.Surround(traceTag, endLine);
	Print(str, 8);
#endif
}

template<typename Types> inline void Logger::Fatal(const Types& arg)
{
	String str(fatalTag, arg, endLine);
	PrintError(str, 64);
}

template<typename Types> inline void Logger::Error(const Types& arg)
{
	String str(errorTag, arg, endLine);
	PrintError(str, 4);
}

template<typename Types> inline void Logger::Warn(const Types& arg)
{
#if LOG_WARN_ENABLED
	String str(warnTag, arg, endLine);
	Print(str, 6);
#endif
}

template<typename Types> inline void Logger::Info(const Types& arg)
{
#if LOG_INFO_ENABLED
	String str(infoTag, arg, endLine);
	Print(str, 10);
#endif
}

template<typename Types> inline void Logger::Debug(const Types& arg)
{
#if LOG_DEBUG_ENABLED
	String str(debugTag, arg, endLine);
	Print(str, 11);
#endif
}

template<typename Types> inline void Logger::Trace(const Types& arg)
{
#if LOG_TRACE_ENABLED
	String str(traceTag, arg, endLine);
	Print(str, 8);
#endif
}
