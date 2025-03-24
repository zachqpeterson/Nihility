#pragma once

#include "Defines.hpp"

#include "File.hpp"

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
class Logger
{
public:
	template<typename... Args> NH_API static void Debug(Args&&... args)
	{
#if LOG_DEBUG_ENABLED == 1
		console.FormatedWrite("\033[0;36m[DEBUG]:\033[0m ", args..., '\n');
		logFile.FormatedWrite("[DEBUG]: ", args..., '\n');
#endif
	}
	template<typename... Args> NH_API static void Trace(Args&&... args)
	{
#if LOG_TRACE_ENABLED == 1
		console.FormatedWrite("\033[1;30m[TRACE]:\033[0m ", args..., '\n');
		logFile.FormatedWrite("[TRACE]: ", args..., '\n');
#endif
	}
	template<typename... Args> NH_API static void Info(Args&&... args)
	{
#if LOG_INFO_ENABLED == 1
		console.FormatedWrite("\033[1;32m[INFO]:\033[0m  ", args..., '\n');
		logFile.FormatedWrite("[INFO]:  ", args..., '\n');
#endif
	}
	template<typename... Args> NH_API static void Warn(Args&&... args)
	{
#if LOG_WARN_ENABLED == 1
		console.FormatedWrite("\033[1;33m[WARN]:\033[0m  ", args..., '\n');
		logFile.FormatedWrite("[WARN]:  ", args..., '\n');
#endif
	}
	template<typename... Args> NH_API static void Error(Args&&... args)
	{
#if LOG_ERROR_ENABLED == 1
		console.FormatedWrite("\033[0;31m[ERROR]:\033[0m ", args..., '\n');
		logFile.FormatedWrite("[ERROR]: ", args..., '\n');
#endif
	}
	template<typename... Args> NH_API static void Fatal(Args&&... args)
	{
#if LOG_FATAL_ENABLED == 1
		console.FormatedWrite("\033[0;41m[FATAL]:\033[0m ", args..., '\n');
		logFile.FormatedWrite("[FATAL]: ", args..., '\n');
#endif
	}

private:
	static bool Initialize();
	static void Shutdown();

	static File logFile;
	static File console;

	friend class Engine;

	STATIC_CLASS(Logger);
};