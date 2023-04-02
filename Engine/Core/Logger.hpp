#pragma once

#include "Defines.hpp"

#include "Containers\String.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef NH_DEBUG
#	define LOG_DEBUG_ENABLED 1
#	define LOG_TRACE_ENABLED 1
#else
#	define LOG_DEBUG_ENABLED 0
#	define LOG_TRACE_ENABLED 0
#endif

struct File;

class NH_API Logger
{
public:
	template<Character T, typename... Types> static void Fatal(const T* message, const Types&... args);
	template<Character T, typename... Types> static void Error(const T* message, const Types&... args);
	template<Character T, typename... Types> static void Warn(const T* message, const Types&... args);
	template<Character T, typename... Types> static void Info(const T* message, const Types&... args);
	template<Character T, typename... Types> static void Debug(const T* message, const Types&... args);
	template<Character T, typename... Types> static void Trace(const T* message, const Types&... args);
	template<typename Type> static void Fatal(const Type& arg);
	template<typename Type> static void Error(const Type& arg);
	template<typename Type> static void Warn(const Type& arg);
	template<typename Type> static void Info(const Type& arg);
	template<typename Type> static void Debug(const Type& arg);
	template<typename Type> static void Trace(const Type& arg);

private:
	static bool Initialize();
	static void Shutdown();
	static void Write(const String& message);

	static inline String fatalTag{ "\033[0;41m[FATAL]:\033[0m " };
	static inline String errorTag{ "\033[0;31m[ERROR]:\033[0m " };
	static inline String warnTag{ "\033[1;33m[WARN]:\033[0m  " };
	static inline String infoTag{ "\033[1;32m[INFO]:\033[0m  " };
	static inline String debugTag{ "\033[0;36m[DEBUG]:\033[0m " };
	static inline String traceTag{ "\033[1;30m[TRACE]:\033[0m " };
	static inline String endLine{ "\n" };

	STATIC_CLASS(Logger);
	friend class Engine;
};

template<Character T, typename... Types> inline void Logger::Fatal(const T* message, const Types&... args)
{
	String str(message, args...);
	Write(str.Surround(fatalTag, endLine));
}

template<Character T, typename... Types> inline void Logger::Error(const T* message, const Types&... args)
{
	String str(message, args...);
	Write(str.Surround(errorTag, endLine));
}

template<Character T, typename... Types> inline void Logger::Warn(const T* message, const Types&... args)
{
#if LOG_WARN_ENABLED
	String str(message, args...);
	Write(str.Surround(warnTag, endLine));
#endif
}

template<Character T, typename... Types> inline void Logger::Info(const T* message, const Types&... args)
{
#if LOG_INFO_ENABLED
	String str(message, args...);
	Write(str.Surround(infoTag, endLine));
#endif
}

template<Character T, typename... Types> inline void Logger::Debug(const T* message, const Types&... args)
{
#if LOG_DEBUG_ENABLED
	String str(message, args...);
	Write(str.Surround(debugTag, endLine));
#endif
}

template<Character T, typename... Types> inline void Logger::Trace(const T* message, const Types&... args)
{
#if LOG_TRACE_ENABLED
	String str(message, args...);
	Write(str.Surround(traceTag, endLine));
#endif
}

template<typename Type> inline void Logger::Fatal(const Type& arg) { Write(String(fatalTag, arg, endLine)); }

template<typename Type> inline void Logger::Error(const Type& arg) { Write(String(errorTag, arg, endLine)); }

template<typename Type> inline void Logger::Warn(const Type& arg)
{
#if LOG_WARN_ENABLED
	Write(String(warnTag, arg, endLine));
#endif
}

template<typename Type> inline void Logger::Info(const Type& arg)
{
#if LOG_INFO_ENABLED
	Write(String(infoTag, arg, endLine));
#endif
}

template<typename Type> inline void Logger::Debug(const Type& arg)
{
#if LOG_DEBUG_ENABLED
	Write(String(debugTag, arg, endLine));
#endif
}

template<typename Type> inline void Logger::Trace(const Type& arg)
{
#if LOG_TRACE_ENABLED
	Write(String(traceTag, arg, endLine));
#endif
}
