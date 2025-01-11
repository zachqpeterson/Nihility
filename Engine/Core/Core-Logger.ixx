module;

#include "Defines.hpp"
#include "TypeTraits.hpp"

export module Core:Logger;

import Containers;

#define ENABLE_LOGGER_QUEUE 0

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef NH_DEBUG
#	define LOG_DEBUG_ENABLED 1
#	define LOG_TRACE_ENABLED 1
#else
#	define LOG_DEBUG_ENABLED 0
#	define LOG_TRACE_ENABLED 0
#endif

#define FATAL_TAG "\033[0;41m[FATAL]:\033[0m "
#define ERROR_TAG "\033[0;31m[ERROR]:\033[0m "
#define WARN_TAG "\033[1;33m[WARN]:\033[0m  "
#define INFO_TAG "\033[1;32m[INFO]:\033[0m  "
#define DEBUG_TAG "\033[0;36m[DEBUG]:\033[0m "
#define TRACE_TAG "\033[1;30m[TRACE]:\033[0m "
#define END_LINE "\n"

export class NH_API Logger
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

	static void Write(String&& message) noexcept;

	static void Queue(String&& message) noexcept;
	static void Output();

	static SafeQueue<String, 64> messageQueue;
	static bool writing;

	STATIC_CLASS(Logger);
	friend class Engine;
};

#if ENABLE_LOGGER_QUEUE
#define OUTPUT Queue
#else
#define OUTPUT Write
#endif


template<Character T, typename... Types> inline void Logger::Fatal(const T* message, const Types&... args)
{
	//Write({ FATAL_TAG, Format{ message, args... }, END_LINE }); TODO: Make this possible
	String str{ FATAL_TAG };

	str.Format(CountOf(FATAL_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
}

template<Character T, typename... Types> inline void Logger::Error(const T* message, const Types&... args)
{
	String str{ ERROR_TAG };

	str.Format(CountOf(ERROR_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
}

template<Character T, typename... Types> inline void Logger::Warn(const T* message, const Types&... args)
{
#if LOG_WARN_ENABLED
	String str{ WARN_TAG };

	str.Format(CountOf(WARN_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
#endif
}

template<Character T, typename... Types> inline void Logger::Info(const T* message, const Types&... args)
{
#if LOG_INFO_ENABLED
	String str{ INFO_TAG };

	str.Format(CountOf(INFO_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
#endif
}

template<Character T, typename... Types> inline void Logger::Debug(const T* message, const Types&... args)
{
#if LOG_DEBUG_ENABLED
	String str{ DEBUG_TAG };

	str.Format(CountOf(DEBUG_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
#endif
}

template<Character T, typename... Types> inline void Logger::Trace(const T* message, const Types&... args)
{
#if LOG_TRACE_ENABLED
	String str{ TRACE_TAG };

	str.Format(CountOf(TRACE_TAG), message, args...);

	OUTPUT(Move(str.Append(END_LINE)));
#endif
}

template<typename Type> inline void Logger::Fatal(const Type& arg)
{
	OUTPUT(Move(String(FATAL_TAG, arg, END_LINE)));
}

template<typename Type> inline void Logger::Error(const Type& arg)
{
	OUTPUT(Move(String(ERROR_TAG, arg, END_LINE)));
}

template<typename Type> inline void Logger::Warn(const Type& arg)
{
#if LOG_WARN_ENABLED
	OUTPUT(Move(String(WARN_TAG, arg, END_LINE)));
#endif
}

template<typename Type> inline void Logger::Info(const Type& arg)
{
#if LOG_INFO_ENABLED
	OUTPUT(Move(String(INFO_TAG, arg, END_LINE)));
#endif
}

template<typename Type> inline void Logger::Debug(const Type& arg)
{
#if LOG_DEBUG_ENABLED
	OUTPUT(Move(String(DEBUG_TAG, arg, END_LINE)));
#endif
}

template<typename Type> inline void Logger::Trace(const Type& arg)
{
#if LOG_TRACE_ENABLED
	OUTPUT(Move(String(TRACE_TAG, arg, END_LINE)));
#endif
}
