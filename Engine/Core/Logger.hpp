#pragma once

#include "Defines.hpp"
#include "File.hpp"

#include "Containers\String.hpp"
#include "Platform\Platform.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef NH_DEBUG
#	define LOG_DEBUG_ENABLED 1
#	define LOG_TRACE_ENABLED 1
#else
#	define LOG_DEBUG_ENABLED 0
#	define LOG_TRACE_ENABLED 0
#endif

struct String;
struct File;

class NH_API Logger
{
public:
	template<typename... Types> static void Fatal(const char* message, const Types&... args);
	template<typename... Types> static void Error(const char* message, const Types&... args);
	template<typename... Types> static void Warn(const char* message, const Types&... args);
	template<typename... Types> static void Info(const char* message, const Types&... args);
	template<typename... Types> static void Debug(const char* message, const Types&... args);
	template<typename... Types> static void Trace(const char* message, const Types&... args);
	template<typename Type> static void Fatal(const Type& arg);
	template<typename Type> static void Error(const Type& arg);
	template<typename Type> static void Warn(const Type& arg);
	template<typename Type> static void Info(const Type& arg);
	template<typename Type> static void Debug(const Type& arg);
	template<typename Type> static void Trace(const Type& arg);

private:
	static bool Initialize();
	static void Shutdown();

	static inline const String fatalTag{ "\033[0;41m[FATAL]:\033[0m " };
	static inline const String errorTag{ "\033[0;31m[ERROR]:\033[0m " };
	static inline const String warnTag{ "\033[1;33m[WARN]:\033[0m  " };
	static inline const String infoTag{ "\033[1;32m[INFO]:\033[0m  " };
	static inline const String debugTag{ "\033[0;36m[DEBUG]:\033[0m " };
	static inline const String traceTag{ "\033[1;30m[TRACE]:\033[0m " };
	static inline const String endLine{ "\n" };

	static inline File log{ "Log.log", FILE_OPEN_WRITE_NEW };
	static inline File console{ "CONOUT$", FILE_OPEN_CONSOLE }; //TODO: This is platform specific

	STATIC_CLASS(Logger);
	friend class Engine;
};

inline bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle(L"Nihility Console");

	return true;
}

inline void Logger::Shutdown()
{
	log.Close();
}

template<typename... Types> inline void Logger::Fatal(const char* message, const Types&... args)
{
	String str(message, args...);
	str.Surround(fatalTag, endLine);
	console.Write(str);
	log.Write(str);
}

template<typename... Types> inline void Logger::Error(const char* message, const Types&... args)
{
	String str(message, args...);
	str.Surround(errorTag, endLine);
	console.Write(str);
	log.Write(str);
}

template<typename... Types> inline void Logger::Warn(const char* message, const Types&... args)
{
#if LOG_WARN_ENABLED
	String str(message, args...);
	str.Surround(warnTag, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename... Types> inline void Logger::Info(const char* message, const Types&... args)
{
#if LOG_INFO_ENABLED
	String str(message, args...);
	str.Surround(infoTag, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename... Types> inline void Logger::Debug(const char* message, const Types&... args)
{
#if LOG_DEBUG_ENABLED
	String str(message, args...);
	str.Surround(debugTag, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename... Types> inline void Logger::Trace(const char* message, const Types&... args)
{
#if LOG_TRACE_ENABLED
	String str(message, args...);
	str.Surround(traceTag, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename Type> inline void Logger::Fatal(const Type& arg)
{
	String str(fatalTag, arg, endLine);
	console.Write(str);
	log.Write(str);
}

template<typename Type> inline void Logger::Error(const Type& arg)
{
	String str(errorTag, arg, endLine);
	console.Write(str);
	log.Write(str);
}

template<typename Type> inline void Logger::Warn(const Type& arg)
{
#if LOG_WARN_ENABLED
	String str(warnTag, arg, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename Type> inline void Logger::Info(const Type& arg)
{
#if LOG_INFO_ENABLED
	String str(infoTag, arg, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename Type> inline void Logger::Debug(const Type& arg)
{
#if LOG_DEBUG_ENABLED
	String str(debugTag, arg, endLine);
	console.Write(str);
	log.Write(str);
#endif
}

template<typename Type> inline void Logger::Trace(const Type& arg)
{
#if LOG_TRACE_ENABLED
	String str(traceTag, arg, endLine);
	console.Write(str);
	log.Write(str);
#endif
}
