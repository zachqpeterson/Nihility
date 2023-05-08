#pragma once

/*---------DATA TYPES---------*/

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
typedef const char* CSTR;		//C-style string

typedef decltype(__nullptr) NullPointer; //Nullptr type

static inline constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFFULL;	//Maximum value of an unsigned 64-bit integer
static inline constexpr U64 U64_MIN = 0x0000000000000000ULL;	//Minimum value of an unsigned 64-bit integer
static inline constexpr I64 I64_MAX = 0x7FFFFFFFFFFFFFFFLL;		//Maximum value of a signed 64-bit integer
static inline constexpr I64 I64_MIN = 0x8000000000000000LL;		//Minimum value of a signed 64-bit integer
static inline constexpr U32 U32_MAX = 0xFFFFFFFFU;				//Maximum value of an unsigned 32-bit integer
static inline constexpr U32 U32_MIN = 0x00000000U;				//Minimum value of an unsigned 32-bit integer
static inline constexpr I32 I32_MAX = 0x7FFFFFFFI32;			//Maximum value of a signed 32-bit integer
static inline constexpr I32 I32_MIN = 0x80000000I32;			//Minimum value of a signed 32-bit integer
static inline constexpr UL32 UL32_MAX = 0xFFFFFFFFUL;			//Maximum value of an unsigned 32-bit integer
static inline constexpr UL32 UL32_MIN = 0x00000000UL;			//Minimum value of an unsigned 32-bit integer
static inline constexpr L32 L32_MAX = 0x7FFFFFFFL;				//Maximum value of a signed 32-bit integer
static inline constexpr L32 L32_MIN = 0x80000000L;				//Minimum value of a signed 32-bit integer
static inline constexpr U16 U16_MAX = 0xFFFFUI16;				//Maximum value of an unsigned 16-bit integer
static inline constexpr U16 U16_MIN = 0x0000UI16;				//Minimum value of an unsigned 16-bit integer
static inline constexpr I16 I16_MAX = 0x7FFFI16;				//Maximum value of a signed 16-bit integer
static inline constexpr I16 I16_MIN = 0x8000I16;				//Minimum value of a signed 16-bit integer
static inline constexpr U8 U8_MAX = 0xFFUI8;					//Maximum value of an unsigned 8-bit integer
static inline constexpr U8 U8_MIN = 0x00UI8;					//Minimum value of an unsigned 8-bit integer
static inline constexpr I8 I8_MAX = 0x7FI8;						//Maximum value of a signed 8-bit integer
static inline constexpr I8 I8_MIN = 0x80I8;						//Minimum value of a signed 8-bit integer
static inline constexpr F32 F32_MAX = 3.402823466e+38F;			//Maximum value of a 32-bit float
static inline constexpr F32 F32_MIN = 1.175494351e-38F;			//Minimum value of a 32-bit float
static inline constexpr F64 F64_MAX = 1.7976931348623158e+308;	//Maximum value of a 64-bit float
static inline constexpr F64 F64_MIN = 2.2250738585072014e-308;	//Minimum value of a 64-bit float

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

#if defined PLATFORM_WINDOWS || defined __LITTLE_ENDIAN__ || (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define NH_LITTLE_ENDIAN
#elif defined __BIG_ENDIAN__ || (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define NH_BIG_ENDIAN
#else
#warning "could not determine endianness! Falling back to little endian..."
#define NH_LITTLE_ENDIAN
#endif

/*---------PLATFORM MACROS---------*/

#ifdef _DEBUG
#	define NH_DEBUG				// Defined if running in debug mode
#	define ASSERTIONS_ENABLED	// Defined if assertions are to be enabled
#else
#	define NH_RELEASE			// Defined if running in release mode
#endif

#ifdef NH_EXPORT
#	ifdef _MSC_VER
#		define NH_API __declspec(dllexport)						// Marks a function or class to be exported
#	else
#		define NH_API __attribute__((visibility("default")))	// Marks a function or class to be exported
#	endif
#else
#	ifdef _MSC_VER
#		define NH_API __declspec(dllimport)	// Marks a function or class to be imported
#	else
#		define NH_API						// Marks a function or class to be imported
#	endif
#endif

#define FUNCTION_NAME __FUNCTION__	// Replaced by the name of this function ex. Class::Function
#define FILE_NAME __FILE__			// Replaced by the name of this file ex. C:/file.cpp
#define LINE_NUMBER __LINE__		// Replaced by the line number ex. 123

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
#	define NH_INLINE __attribute__((always_inline)) inline	// Tries to force the compiler to inline a function
#	define NH_NOINLINE __attribute__((noinline))			// Tries to force the compiler to not inline a function
#elif defined _MSC_VER
#	define NH_INLINE __forceinline							// Tries to force the compiler to inline a function
#	define NH_NOINLINE __declspec(noinline)					// Tries to force the compiler to not inline a function
#else
#	define NH_INLINE static inline							// Tries to force the compiler to inline a function
#	define NH_NOINLINE										// Tries to force the compiler to not inline a function
#endif

#define NH_HEADER_STATIC static inline	// A static member variable that doesn't get defined in a cpp file
#define NH_NODISCARD [[nodiscard]]	// Issues a warning when the return value of a function isn't captured

/*---------ASSERTIONS---------*/

#ifdef ASSERTIONS_ENABLED
#	if _MSC_VER
#		include <intrin.h>
#		define BreakPoint __debugbreak()	// Halts the execution of the program when reached
#	else
#		define BreakPoint __builtin_trap()	// Halts the execution of the program when reached
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

/// <summary>
/// Gets the next power of 2 from a starting point
/// </summary>
/// <param name="value:">The starting value</param>
/// <returns></returns>
inline constexpr I64 NextPow2(const I64& value)
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

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U64 Count> inline constexpr U64 CountOf(Type(&)[Count]) { return Count; }

/// <summary>
/// Gets the element count of a static array
/// </summary>
/// <returns>The count of elements</returns>
template<class Type, U64 Count> inline constexpr U32 CountOf32(Type(&)[Count]) { return (U32)Count; }

#include "TypeTraits.hpp"