#pragma once

/*---------DATA TYPES---------*/

///Unsigned 8-bit integer
typedef unsigned char U8;

///Unsigned 16-bit integer

typedef unsigned short U16;

///Unsigned 32-bit integer
typedef unsigned long U32;

///Unsigned 64-bit integer
typedef unsigned long long U64;

///Signed 8-bit integer
typedef signed char I8;

///Signed 16-bit integer
typedef signed short I16;

///Signed 32-bit integer
typedef signed long I32;

///Signed 64-bit integer
typedef signed long long I64;

///32-bit floating point number
typedef float F32;

///64-bit floating point number
typedef double F64;

///128-bit floating point number
typedef long double F128;

///Maximum value of an unsigned 64-bit integer
#define U64_MAX 0xFFFFFFFFFFFFFFFFUI64

///Maximum value of a signed 64-bit integer
#define I64_MAX 0x7FFFFFFFFFFFFFFFI64

///Minimum value of a signed 64-bit integer
#define I64_MIN 0x8000000000000000I64

///Maximum value of an unsigned 32-bit integer
#define U32_MAX 0xFFFFFFFFUI32

///Maximum value of a signed 32-bit integer
#define I32_MAX 0x7FFFFFFFI32

///Minimum value of a signed 32-bit integer
#define I32_MIN 0x80000000I32

///Maximum value of an unsigned 16-bit integer
#define U16_MAX 0xFFFFUI16

///Maximum value of a signed 16-bit integer
#define I16_MAX 0x7FFFI16

///Minimum value of a signed 16-bit integer
#define I16_MIN 0x8000I16

///Maximum value of an unsigned 8-bit integer
#define U8_MAX  0xFFUI8

///Maximum value of a signed 8-bit integer
#define I8_MAX  0x7FI8

///Minimum value of a signed 8-bit integer
#define I8_MIN  0x80I8

///Maximum value of a 32-bit float
#define F32_MAX 3.402823466e+38F

///Maximum value of a 64-bit float
#define F64_MAX 1.7976931348623158e+308

///Maximum value of a 128-bit float
#define F128_MAX 1.1897314953572317650857593266280070162e+4932

/*---------PLATFORM DETECTION---------*/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _WIN64 //---WINDOWS
#	define PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define MIN_ALLOCATION_ALIGNMENT 16
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
#define NH_DEBUG
#else
#define NH_RELEASE
#endif

#ifdef NH_EXPORT
#	ifdef _MSC_VER
#		define NH_API __declspec(dllexport)
#	else
#		define NH_API __attribute__((visibility("default")))
#	endif
#else
#	ifdef _MSC_VER
#		define NH_API __declspec(dllimport)
#	else
#		define NH_API
#	endif
#endif

#define FUNCTION_NAME __FUNCTION__
#define FILE_NAME __FILE__
#define LINE_NUMBER __LINE__

#if defined __clang__ || defined __gcc__
#	define NH_INLINE __attribute__((always_inline)) inline
#	define NH_NOINLINE __attribute__((noinline))
#elif defined _MSC_VER
#	define NH_INLINE __forceinline
#	define NH_NOINLINE __declspec(noinline)
#else
#	define NH_INLINE static inline
#	define NH_NOINLINE
#endif

/*---------ASSERTIONS---------*/

#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#	if _MSC_VER
#		include <intrin.h>
#		define debugBreak() __debugbreak()
#	else
#		define debugBreak() __builtin_trap()
#	endif

#define ASSERT(expr) if (!(expr)) { debugBreak(); }

#if defined __clang__ || defined __gcc__
#	define STATIC_ASSERT(expr) _Static_assert(expr)
#else
#	define STATIC_ASSERT(expr) static_assert(expr)
#endif

#else
#	define ASSERT(expr)
#	define STATIC_ASSERT(expr)
#endif

/*---------MEMORY---------*/

/// <summary>
/// Converts a value to a right value reference
/// </summary>
/// <param name="t:">The value to convert</param>
/// <returns>The converted value</returns>
template<typename T> constexpr T&& Move(T&& t) noexcept { return static_cast<T&&>(t); }

/// <summary>
/// Converts a value to a right value reference
/// </summary>
/// <param name="t:">The value to convert</param>
/// <returns>The converted value</returns>
template<typename T> constexpr T&& Move(T& t) noexcept { return static_cast<T&&>(t); }

/// <summary>
/// Sets value to the next 
/// </summary>
/// <param name="value:"></param>
/// <param name="alignment:"></param>
#define AlignPow2(value, alignment) ((value + (alignment - 1)) & ~(alignment - 1))

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
#define Align4(value) ((value + 3) & ~3)

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
#define Align8(value) ((value + 7) & ~7)

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
#define Align16(value) ((value + 15) & ~15)