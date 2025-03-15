#pragma once

#include "Defines.hpp"

#include "fast_io/fast_io.h"
#include "fast_io/fast_io_device.h"

#ifndef LOG_DEBUG_ENABLED
#	ifdef NH_DEBUG
#		define LOG_DEBUG_ENABLED 1
#	else
#		define LOG_DEBUG_ENABLED 0
#	endif
#endif

#ifndef LOG_TRACE_ENABLED
#	ifdef NH_DEBUG
#		define LOG_TRACE_ENABLED 1
#	else
#		define LOG_TRACE_ENABLED 0
#	endif
#endif

#ifndef LOG_INFO_ENABLED
#	define LOG_INFO_ENABLED 1
#endif

#ifndef LOG_WARN_ENABLED
#	define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_ERROR_ENABLED
#	define LOG_ERROR_ENABLED 1
#endif

#ifndef LOG_FATAL_ENABLED
#	define LOG_FATAL_ENABLED 1
#endif

//TODO: Timestamps
//TODO: Threadsafe
class NH_API Logger
{
public:
	template<typename... Args> static void Debug(Args&&... args)
	{
#if LOG_DEBUG_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[0;36m[DEBUG]:\033[0m ", args...);
		fast_io::io::println(log, "[DEBUG]: ", args...);
#endif
	}
	template<typename... Args> static void Trace(Args&&... args)
	{
#if LOG_TRACE_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[1;30m[TRACE]:\033[0m ", args...);
		fast_io::io::println(log,  "[TRACE]: ", args...);
#endif
	}
	template<typename... Args> static void Info(Args&&... args)
	{
#if LOG_INFO_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[1;32m[INFO]:\033[0m  ", args...);
		fast_io::io::println(log, "[INFO]:  ", args...);
#endif
	}
	template<typename... Args> static void Warn(Args&&... args)
	{
#if LOG_WARN_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[1;33m[WARN]:\033[0m  ", args...);
		fast_io::io::println(log, "[WARN]:  ", args...);
#endif
	}
	template<typename... Args> static void Error(Args&&... args)
	{
#if LOG_ERROR_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[0;31m[ERROR]:\033[0m ", args...);
		fast_io::io::println(log, "[ERROR]: ", args...);
#endif
	}
	template<typename... Args> static void Fatal(Args&&... args)
	{
#if LOG_FATAL_ENABLED == 1
		fast_io::io::println(fast_io::out(), "\033[0;41m[FATAL]:\033[0m ", args...);
		fast_io::io::println(log, "[FATAL]: ", args...);
#endif
	}

private:
	static bool Initialize();
	static void Shutdown();

	static fast_io::obuf_file log;

	friend class Engine;

	STATIC_CLASS(Logger);
};