#pragma once

typedef unsigned char U8;		//Unsigned 8-bit integer
typedef unsigned short U16;		//Unsigned 16-bit integer
typedef unsigned int U32;		//Unsigned 32-bit integer
typedef unsigned long UL32;		//Unsigned 32-bit integer
typedef unsigned long long U64;	//Unsigned 64-bit integer

typedef signed char I8;			//Signed 8-bit integer
typedef signed short I16;		//Signed 16-bit integer
typedef signed int I32;			//Signed 32-bit integer
typedef signed long L32;		//Signed 32-bit integer
typedef signed long long I64;	//Signed 64-bit integer

typedef float F32;				//32-bit floating point number
typedef double F64;				//64-bit floating point number

typedef char C8;				//8-bit ascii character
typedef char16_t C16;			//16-bit unicode character
typedef wchar_t CW;				//Platform defined wide character, WINDOWS: 16-bit, OTHER: 32-bit
typedef char32_t C32;			//32-bit unicode character

typedef decltype(__nullptr) NullPointer; //Nullptr type

static inline constexpr unsigned long long U64_MAX = 0xFFFFFFFFFFFFFFFFULL;	//Maximum value of an unsigned 64-bit integer
static inline constexpr unsigned long long U64_MIN = 0x0000000000000000ULL;	//Minimum value of an unsigned 64-bit integer
static inline constexpr signed long long I64_MAX = 0x7FFFFFFFFFFFFFFFLL;	//Maximum value of a signed 64-bit integer
static inline constexpr signed long long I64_MIN = 0x8000000000000000LL;	//Minimum value of a signed 64-bit integer
static inline constexpr unsigned int U32_MAX = 0xFFFFFFFFU;					//Maximum value of an unsigned 32-bit integer
static inline constexpr unsigned int U32_MIN = 0x00000000U;					//Minimum value of an unsigned 32-bit integer
static inline constexpr signed int I32_MAX = 0x7FFFFFFFI32;					//Maximum value of a signed 32-bit integer
static inline constexpr signed int I32_MIN = 0x80000000I32;					//Minimum value of a signed 32-bit integer
static inline constexpr unsigned long UL32_MAX = 0xFFFFFFFFUL;				//Maximum value of an unsigned 32-bit integer
static inline constexpr unsigned long UL32_MIN = 0x00000000UL;				//Minimum value of an unsigned 32-bit integer
static inline constexpr signed long L32_MAX = 0x7FFFFFFFL;					//Maximum value of a signed 32-bit integer
static inline constexpr signed long L32_MIN = 0x80000000L;					//Minimum value of a signed 32-bit integer
static inline constexpr unsigned short U16_MAX = 0xFFFFUI16;				//Maximum value of an unsigned 16-bit integer
static inline constexpr unsigned short U16_MIN = 0x0000UI16;				//Minimum value of an unsigned 16-bit integer
static inline constexpr signed short I16_MAX = 0x7FFFI16;					//Maximum value of a signed 16-bit integer
static inline constexpr signed short I16_MIN = 0x8000I16;					//Minimum value of a signed 16-bit integer
static inline constexpr unsigned char U8_MAX = 0xFFUI8;						//Maximum value of an unsigned 8-bit integer
static inline constexpr unsigned char U8_MIN = 0x00UI8;						//Minimum value of an unsigned 8-bit integer
static inline constexpr signed char I8_MAX = 0x7FI8;						//Maximum value of a signed 8-bit integer
static inline constexpr signed char I8_MIN = 0x80I8;						//Minimum value of a signed 8-bit integer
static inline constexpr float F32_MAX = 3.402823466e+38F;					//Maximum value of a 32-bit float
static inline constexpr float F32_MIN = 1.175494351e-38F;					//Minimum value of a 32-bit float
static inline constexpr double F64_MAX = 1.7976931348623158e+308;			//Maximum value of a 64-bit float
static inline constexpr double F64_MIN = 2.2250738585072014e-308;			//Minimum value of a 64-bit float

#if defined (WIN32) || defined (_WIN32) || defined (__WIN32__) || defined (__NT__)
#	define NH_PLATFORM_WINDOWS //Defined when on a Windows operating system
#	ifndef _WIN64
#		error "64-bit operating system is required!"
#	endif
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#	define NH_PLATFORM_IOS_SIM //Defined when on an IOS simulator
#	define NH_PLATFORM_IOS //Defined when on an IOS
#elif TARGET_OS_MACCATALYST
#	define NH_PLATFORM_MACCATALYST //Defined when on an IOS
#elif TARGET_OS_IPHONE
#	define NH_PLATFORM_IPHONE //Defined when on an IOS
#elif TARGET_OS_MAC
#	define NH_PLATFORM_MAC //Defined when on an mac operating system
#else
#   error "Unknown Apple platform"
#endif
#elif __ANDROID__
#	define NH_PLATFORM_ANDROID //Defined when on an Android
#elif __linux__
#	define NH_PLATFORM_LINUX //Defined when on a Linux operating system
#elif __unix__
#	define NH_PLATFORM_UNIX //Defined when on a Unix operating system
#elif defined (_POSIX_VERSION)
#	define NH_PLATFORM_POSIX //Defined when on a Posix operating system
#else
#   error "Unknown compiler"
#endif

#if defined (NH_PLATFORM_WINDOWS) || defined (__LITTLE_ENDIAN__) || (defined (__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#	define NH_LITTLE_ENDIAN  //Defined when on a little endian operating system
#elif defined (__BIG_ENDIAN__) || (defined (__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#	define NH_BIG_ENDIAN //Defined when on a big endian operating system
#else
#	warning "could not determine endianness! Falling back to little endian..."
#	define NH_LITTLE_ENDIAN //Defined when on a little endian operating system
#endif

#ifdef _DEBUG
#	define NH_DEBUG				// Defined if running in debug mode
#	define ASSERTIONS_ENABLED	// Defined if assertions are enabled
#else
#	define NH_RELEASE			// Defined if running in release mode
#endif

#ifdef NH_EXPORT
#	define NH_API __declspec(dllexport)						// Marks a function or class to be exported
#else
#	define NH_API __declspec(dllimport)						// Marks a function or class to be imported
#endif

#define FUNCTION_NAME __FUNCTION__	// Replaced by the name of this function ex. namespace::Class::Function<template args>
#define FILE_NAME __FILE__			// Replaced by the name of this file ex. C:/file.cpp
#define LINE_NUMBER __LINE__		// Replaced by the line number ex. 123

#define NH_INLINE __forceinline				// Tries to force the compiler to inline a function
#define NH_NOINLINE __declspec(noinline)	// Tries to force the compiler to not inline a function

#define NH_NODISCARD [[nodiscard]]							// Issues a warning when the return value of a function isn't captured
#define NH_NODISCARD_MSG(message) [[nodiscard(message)]]	// Issues a warning when the return value of a function isn't captured

#include <intrin.h>

#ifdef ASSERTIONS_ENABLED
#	define BreakPoint __debugbreak()	// Halts the execution of the program when reached

/// <summary>
/// Halts the execution of the program if expr is false
/// </summary>
/// <param name="expr:">The expression to check</param>
#	define ASSERT(expr) if (expr) { } else { BreakPoint; }

#else
#	define BreakPoint
#	define ASSERT(expr) expr;
#endif

/// <summary>
/// Deletes a class's constructor, assignment operators, and destructor
/// </summary>
/// <param name="class:">The class to make static</param>
#define STATIC_CLASS(class)			\
class() = delete;					\
~class() = delete;					\
class(class&) = delete;				\
class(class&&) = delete;			\
class& operator=(class&) = delete;	\
class& operator=(class&&) = delete;	

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U64 Count> constexpr U64 CountOf(Type(&)[Count]) { return Count; }

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U32 Count> constexpr U32 CountOf32(Type(&)[Count]) { return Count; }

/// <summary>
/// Creates a number that represents a version
/// </summary>
/// <param name="major:">The major version</param>
/// <param name="minor:">The minor version</param>
/// <param name="patch:">The patch version</param>
/// <returns>The version number</returns>
constexpr U32 MakeVersionNumber(U8 major, U8 minor, U8 patch)
{
	return (major << 16) | (minor << 8) | patch;
}

/// <summary>
/// Extracts the patch number from a version number
/// </summary>
/// <param name="versionNumber:">The version number to get the patch number from</param>
/// <returns>The patch number</returns>
constexpr U32 GetPatchVersion(U32 versionNumber)
{
	constexpr U32 PATCH_MASK = 0b11111111;
	return versionNumber & PATCH_MASK;
}

/// <summary>
/// Extracts the minor number from a version number
/// </summary>
/// <param name="versionNumber:">The version number to get the minor number from</param>
/// <returns>The minor number</returns>
constexpr U32 GetMinorVersion(U32 versionNumber)
{
	constexpr U32 MINOR_MASK = 0b1111111100000000;
	return (versionNumber & MINOR_MASK) >> 8;
}

/// <summary>
/// Extracts the major number from a version number
/// </summary>
/// <param name="versionNumber:">The version number to get the major number from</param>
/// <returns>The major number</returns>
constexpr U32 GetMajorVersion(U32 versionNumber)
{
	constexpr U32 MAJOR_MASK = 0b111111110000000000000000;
	return (versionNumber & MAJOR_MASK) >> 16;
}