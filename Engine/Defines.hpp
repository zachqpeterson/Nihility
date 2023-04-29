#pragma once

//TODO: Separate this into different sections i.e. Platform, Resources, Containers, Math, etc.

/*---------DATA TYPES---------*/

///Unsigned 8-bit integer
typedef unsigned char U8;

///Unsigned 16-bit integer
typedef unsigned short U16;

///Unsigned 32-bit integer
typedef unsigned int U32;

///Unsigned 32-bit integer
typedef unsigned long UL32;

///Unsigned 64-bit integer
typedef unsigned long long U64;

///Signed 8-bit integer
typedef signed char I8;

///Signed 16-bit integer
typedef signed short I16;

///Signed 32-bit integer
typedef signed int I32;

///Signed 32-bit integer
typedef signed long L32;

///Signed 64-bit integer
typedef signed long long I64;

///32-bit floating point number
typedef float F32;

///64-bit floating point number
typedef double F64;

///8-bit ascii character
typedef char C8;

///16-bit unicode character
typedef char16_t C16;

///16-bit unicode character
typedef wchar_t CW;

///16-bit unicode character
typedef char32_t C32;

typedef const char* CSTR;

typedef decltype(__nullptr) NullPointer;

///Maximum value of an unsigned 64-bit integer
static constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFFULL;

///Minimum value of an unsigned 64-bit integer
static constexpr U64 U64_MIN = 0x0000000000000000ULL;

///Maximum value of a signed 64-bit integer
static constexpr I64 I64_MAX = 0x7FFFFFFFFFFFFFFFLL;

///Minimum value of a signed 64-bit integer
static constexpr I64 I64_MIN = 0x8000000000000000LL;

///Maximum value of an unsigned 32-bit integer
static constexpr U32 U32_MAX = 0xFFFFFFFFU;

///Minimum value of an unsigned 32-bit integer
static constexpr U32 U32_MIN = 0x00000000U;

///Maximum value of a signed 32-bit integer
static constexpr I32 I32_MAX = 0x7FFFFFFFI32;

///Minimum value of a signed 32-bit integer
static constexpr I32 I32_MIN = 0x80000000I32;

///Maximum value of an unsigned 32-bit integer
static constexpr UL32 UL32_MAX = 0xFFFFFFFFUL;

///Minimum value of an unsigned 32-bit integer
static constexpr UL32 UL32_MIN = 0x00000000UL;

///Maximum value of a signed 32-bit integer
static constexpr L32 L32_MAX = 0x7FFFFFFFL;

///Minimum value of a signed 32-bit integer
static constexpr L32 L32_MIN = 0x80000000L;

///Maximum value of an unsigned 16-bit integer
static constexpr U16 U16_MAX = 0xFFFFUI16;

///Minimum value of an unsigned 16-bit integer
static constexpr U16 U16_MIN = 0x0000UI16;

///Maximum value of a signed 16-bit integer
static constexpr I16 I16_MAX = 0x7FFFI16;

///Minimum value of a signed 16-bit integer
static constexpr I16 I16_MIN = 0x8000I16;

///Maximum value of an unsigned 8-bit integer
static constexpr U8 U8_MAX = 0xFFUI8;

///Minimum value of an unsigned 8-bit integer
static constexpr U8 U8_MIN = 0x00UI8;

///Maximum value of a signed 8-bit integer
static constexpr I8 I8_MAX = 0x7FI8;

///Minimum value of a signed 8-bit integer
static constexpr I8 I8_MIN = 0x80I8;

///Maximum value of a 32-bit float
static constexpr F32 F32_MAX = 3.402823466e+38F;

///Minimum value of a 32-bit float
static constexpr F32 F32_MIN = 1.175494351e-38F;

///Maximum value of a 64-bit float
static constexpr F64 F64_MAX = 1.7976931348623158e+308;

///Minimum value of a 64-bit float
static constexpr F64 F64_MIN = 2.2250738585072014e-308;

/*---------PLATFORM DETECTION---------*/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _WIN64 //---WINDOWS
#	define PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	ifndef _WIN64
#		error "64-bit is required on Windows!"
#	endif

#elif defined __linux__ || defined __gnu_linux__ //-----------------------------LINUX
#	define PLATFORM_LINUX
#	if defined __ANDROID__ //---------------------------------------------------ANDROID
#		define PLATFORM_ANDROID
#	endif

#elif defined __unix__ //-------------------------------------------------------UNIX
#	define PLATFORM_UNIX

#elif defined _POSIX_VERSION_ //------------------------------------------------POSIX
#	define PLATFORM_POSIX

#elif defined __APPLE__ || defined __MACH__ //----------------------------------APPLE
#	define PLATFORM_APPLE
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR //-----------------------------------------------IOS SIMULATOR
#		define PLATFORM_IOS
#		define PLATFORM_IOS_SIMULATOR
#	elif TARGET_OS_IPHONE //----------------------------------------------------IOS
#		define PLATFORM_IOS
#	elif TARGET_OS_MAC
#	else
#		error "Unknown Apple platform!"
#	endif

#else //------------------------------------------------------------------------UNKOWN
#	error "Unknown platform!"
#endif

/*---------PLATFORM MACROS---------*/

#ifdef _DEBUG
	/// <summary>
	/// Defined if running in debug mode
	/// </summary>
#	define NH_DEBUG
#	define ASSERTIONS_ENABLED
#else
	/// <summary>
	/// Defined if running in release mode
	/// </summary>
#	define NH_RELEASE
#endif

#ifdef NH_EXPORT
#	ifdef _MSC_VER
		/// <summary>
		/// Marks a function or class to be exported
		/// </summary>
#		define NH_API __declspec(dllexport)
#	else
		/// <summary>
		/// Marks a function or class to be exported
		/// </summary>
#		define NH_API __attribute__((visibility("default")))
#	endif
#else
#	ifdef _MSC_VER
		/// <summary>
		/// Marks a function or class to be imported
		/// </summary>
#		define NH_API __declspec(dllimport)
#	else
		/// <summary>
		/// Marks a function or class to be imported
		/// </summary>
#		define NH_API
#	endif
#endif

/// <summary>
/// Replaced by the name of this function ex. Class::Function
/// </summary>
#define FUNCTION_NAME __FUNCTION__

/// <summary>
/// Replaced by the name of this file ex. C:/file.cpp
/// </summary>
#define FILE_NAME __FILE__

/// <summary>
/// Replaced by the line number
/// </summary>
#define LINE_NUMBER __LINE__

/// <summary>
/// Deletes a class's constructors, assignment operators, destructor
/// </summary>
/// <param name="class:">The class to operate on</param>
#define STATIC_CLASS(class)			\
class() = delete;					\
~class() = delete;					\
class(class&) = delete;				\
class(class&&) = delete;			\
class& operator=(class&) = delete;	\
class& operator=(class&&) = delete;	\

#if defined __clang__ || defined __gcc__
	/// <summary>
	/// Tries to force the compiler to inline a function
	/// </summary>
#	define NH_INLINE __attribute__((always_inline)) inline

	/// <summary>
	/// Tries to force the compiler to not inline a function
	/// </summary>
#	define NH_NOINLINE __attribute__((noinline))
#elif defined _MSC_VER
	/// <summary>
	/// Tries to force the compiler to inline a function
	/// </summary>
#	define NH_INLINE __forceinline

	/// <summary>
	/// Tries to force the compiler to not inline a function
	/// </summary>
#	define NH_NOINLINE __declspec(noinline)
#else
	/// <summary>
	/// Tries to force the compiler to inline a function
	/// </summary>
#	define NH_INLINE static inline

	/// <summary>
	/// Tries to force the compiler to not inline a function
	/// </summary>
#	define NH_NOINLINE
#endif

#define NH_NODISCARD [[nodiscard]]

/*---------ASSERTIONS---------*/

#ifdef ASSERTIONS_ENABLED
#	if _MSC_VER
#		include <intrin.h>
		/// <summary>
		/// Halts the execution of the program when reached
		/// </summary>
#		define BreakPoint __debugbreak()
#	else
		/// <summary>
		/// Halts the execution of the program when reached
		/// </summary>
#		define BreakPoint __builtin_trap()
#	endif

	/// <summary>
	/// Halts the execution of the program if expr is false
	/// </summary>
	/// <param name="expr:">The expression to check</param>
#	define ASSERT(expr) if (!(expr)) { BreakPoint; }

#else
#	define BreakPoint
#	define ASSERT(expr) expr;
#	define STATIC_ASSERT(expr)
#endif

constexpr I64 NextPow2(const I64& value)
{
	I64 val = value;

	--val;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	++val;

	return val;
}

#include "TypeTraits.hpp"