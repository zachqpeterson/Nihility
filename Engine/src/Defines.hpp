/** @file Defines.hpp */
#pragma once

/** Unsigned 8-bit integer */
typedef unsigned char U8;
/** Unsigned 16-bit integer */
typedef unsigned short U16;
/** Unsigned 32-bit integer */
typedef unsigned int U32;
/** Unsigned 64-bit integer */
typedef unsigned long long U64;

/** Signed 8-bit integer */
typedef signed char I8;
/** Signed 16-bit integer */
typedef signed short I16;
/** Signed 32-bit integer */
typedef signed int I32;
/** Signed 64-bit integer */
typedef signed long long I64;

/** 32-bit floating point number */
typedef float F32;
/** 64-bit floating point number */
typedef double F64;

/** Maximum value of an unsigned 64-bit integer */
#define U64_MAX 0xFFFFFFFFFFFFFFFFUI64
/** Maximum value of a signed 64-bit integer */
#define I64_MAX 0x7FFFFFFFFFFFFFFFI64
/** Minimum value of a signed 64-bit integer */
#define I64_MIN 0x8000000000000000I64

#define U32_MAX 0xFFFFFFFFUI32
#define I32_MAX 0x7FFFFFFFI32
#define I32_MIN 0x80000000I32

#define U16_MAX 0xFFFFUI16
#define I16_MAX 0x7FFFI16
#define I16_MIN 0x8000I16

#define U8_MAX  0xFFUI8
#define I8_MAX  0x7FI8
#define I8_MIN  0x80I8

#define F32_MAX 3.402823466e+38F
#define F32_MIN 1.175494351e-38F

#define F64_MAX 1.7976931348623158e+308
#define F64_MIN 2.2250738585072014e-308

#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

/***************************PLATFORM DETECTION***************************/
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define PLATFORM_POSIX 1
#elif defined(__APPLE__) || defined(__MACH__)
// Apple platforms
#define PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define PLATFORM_IOS 1
#define PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

#ifdef _DEBUG
#define NH_DEBUG
#else
#define NH_RELEASE
#endif

#ifdef NH_EXPORT
// Exports
#ifdef _MSC_VER
#define NH_API __declspec(dllexport)
#else
#define NH_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define NH_API __declspec(dllimport)
#else
#define NH_API
#endif
#endif

/***************************INLINING***************************/
#if defined(__clang__) || defined(__gcc__)
#define NH_INLINE __attribute__((always_inline)) inline
#define NH_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define NH_INLINE __forceinline
#define NH_NOINLINE __declspec(noinline)
#else
#define NH_INLINE static inline
#define NH_NOINLINE
#endif

#define AlignPow2(value, Alignment) ((value + (Alignment - 1)) & ~(Alignment - 1))
#define Align4(value) ((value + 3) & ~3)
#define Align8(value) ((value + 7) & ~7)
#define Align16(value) ((value + 15) & ~15)

/***************************ASSERTS***************************/
#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

void ReportAssertion(const char* expression, const char* message, const char* file, I32 line);

#define ASSERT(expr)                                        \
    {                                                       \
        if (expr) {                                         \
        } else {                                            \
            ReportAssertion(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                   \
        }                                                   \
    }

#define ASSERT_MSG(expr, message)                                   \
    {                                                               \
        if (expr) {                                                 \
        } else {                                                    \
            ReportAssertion(#expr, message, __FILE__, __LINE__);    \
            debugBreak();                                           \
        }                                                           \
    }

#ifdef NH_DEBUG
#define ASSERT_DEBUG(expr)                                  \
    {                                                       \
        if (expr) {                                         \
        } else {                                            \
            ReportAssertion(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                   \
        }                                                   \
    }

#define ASSERT_DEBUG_MSG(expr, message)                             \
    {                                                               \
        if (expr) {                                                 \
        } else {                                                    \
            ReportAssertion(#expr, message, __FILE__, __LINE__);    \
            debugBreak();                                           \
        }                                                           \
    }
#else
#define ASSERT_DEBUG(expr)
#define ASSERT_DEBUG_MSG(expr, message)
#endif

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr, message)
#define ASSERT_DEBUG(expr)
#define ASSERT_DEBUG_MSG(expr, message)
#endif

/***************************MOVE DEFINITION***************************/
//Note: This is a simple replacement for std::move
template<typename T>
constexpr T&& move(T&& t) noexcept
{
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& move(T& t) noexcept
{
    return static_cast<T&&>(t);
}