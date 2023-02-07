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

class Logger
{
public:
	template<typename... Types> static void Fatal(const char* message, const Types&... args);
	template<typename... Types> static void Error(const char* message, const Types&... args);
	template<typename... Types> static void Warn(const char* message, const Types&... args);
	template<typename... Types> static void Debug(const char* message, const Types&... args);
	template<typename... Types> static void Info(const char* message, const Types&... args);
	template<typename... Types> static void Trace(const char* message, const Types&... args);
	template<typename Types> static void Fatal(const Types& args);
	template<typename Types> static void Error(const Types& args);
	template<typename Types> static void Warn(const Types& args);
	template<typename Types> static void Debug(const Types& args);
	template<typename Types> static void Info(const Types& args);
	template<typename Types> static void Trace(const Types& args);

private:
	static bool Initialize();
	static void Shutdown();

	STATIC_CLASS(Logger);
	friend class Engine;
};

inline bool Logger::Initialize()
{
	return true;
}

inline void Logger::Shutdown()
{

}

template<typename... Types> inline void Logger::Fatal(const char* message, const Types&... args)
{

}

template<typename... Types> inline void Logger::Error(const char* message, const Types&... args)
{

}

template<typename... Types> inline void Logger::Warn(const char* message, const Types&... args)
{
#if LOG_WARN_ENABLED

#endif
}

template<typename... Types> inline void Logger::Info(const char* message, const Types&... args)
{
#if LOG_INFO_ENABLED

#endif
}

template<typename... Types> inline void Logger::Debug(const char* message, const Types&... args)
{
#if LOG_DEBUG_ENABLED

#endif
}

template<typename... Types> inline void Logger::Trace(const char* message, const Types&... args)
{
#if LOG_TRACE_ENABLED

#endif
}

template<typename Types> inline void Logger::Fatal(const Types& args)
{

}

template<typename Types> inline void Logger::Error(const Types& args)
{

}

template<typename Types> inline void Logger::Warn(const Types& args)
{
#if LOG_WARN_ENABLED

#endif
}

template<typename Types> inline void Logger::Info(const Types& args)
{
#if LOG_INFO_ENABLED

#endif
}

template<typename Types> inline void Logger::Debug(const Types& args)
{
#if LOG_DEBUG_ENABLED

#endif
}

template<typename Types> inline void Logger::Trace(const Types& args)
{
#if LOG_TRACE_ENABLED

#endif
}
