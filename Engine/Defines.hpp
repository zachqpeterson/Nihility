#pragma once

/*---------DATA TYPES---------*/

///Unsigned 8-bit integer
typedef unsigned char U8;

///16-bit unicode character
typedef wchar_t W16;

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

///Maximum value of an unsigned 64-bit integer
static constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFFUI64;

///Maximum value of a signed 64-bit integer
static constexpr I64 I64_MAX = 0x7FFFFFFFFFFFFFFFI64;

///Minimum value of a signed 64-bit integer
static constexpr I64 I64_MIN = 0x8000000000000000I64;

///Maximum value of an unsigned 32-bit integer
static constexpr I64 U32_MAX = 0xFFFFFFFFUI32;

///Maximum value of a signed 32-bit integer
static constexpr I32 I32_MAX = 0x7FFFFFFFI32;

///Minimum value of a signed 32-bit integer
static constexpr I32 I32_MIN = 0x80000000I32;

///Maximum value of an unsigned 16-bit integer
static constexpr U16 U16_MAX = 0xFFFFUI16;

///Maximum value of a signed 16-bit integer
static constexpr I16 I16_MAX = 0x7FFFI16;

///Minimum value of a signed 16-bit integer
static constexpr I16 I16_MIN = 0x8000I16;

///Maximum value of an unsigned 8-bit integer
static constexpr U8 U8_MAX = 0xFFUI8;

///Maximum value of a signed 8-bit integer
static constexpr I8 I8_MAX = 0x7FI8;

///Minimum value of a signed 8-bit integer
static constexpr I8 I8_MIN = 0x80I8;

///Maximum value of a 32-bit float
static constexpr F32 F32_MAX = 3.402823466e+38F;

///Maximum value of a 64-bit float
static constexpr F64 F64_MAX = 1.7976931348623158e+308;

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

#define NH_NO_DISCARD [[nodiscard]]

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

#if defined __clang__ || defined __gcc__
	/// <summary>
	/// Throws a compiler error is expr is false
	/// </summary>
	/// <param name="expr:">The expression to check</param>
#	define STATIC_ASSERT(expr) _Static_assert(expr)
#else
	/// <summary>
	/// Throws a compiler error is expr is false
	/// </summary>
	/// <param name="expr:">The expression to check</param>
#	define STATIC_ASSERT(expr) static_assert(expr)
#endif

#else
#	define BreakPoint
#	define ASSERT(expr) expr;
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
/// Sets value to the next multiple of 2^alignment
/// </summary>
/// <param name="value:">The value to set</param>
/// <param name="alignment:">The power of 2 to align to</param>
#define AlignPow2(value, alignment) ((value + (alignment - 1)) & ~(alignment - 1))

/// <summary>
/// Sets value to the next multiple of 4
/// </summary>
/// <param name="value:">The value to set</param>
#define Align4(value) ((value + 3) & ~3)

/// <summary>
/// Sets value to the next multiple of 8
/// </summary>
/// <param name="value:">The value to set</param>
#define Align8(value) ((value + 7) & ~7)

/// <summary>
/// Sets value to the next multiple of 16
/// </summary>
/// <param name="value:">The value to set</param>
#define Align16(value) ((value + 15) & ~15)

#include <type_traits>

template <typename Type, typename Return = void>
using EnableSignedInt = std::enable_if_t<std::is_signed_v<Type> && std::_Is_nonbool_integral<Type>, Return>;

template <typename Type, typename Return = void>
using EnableUnsignedInt = std::enable_if_t<std::is_unsigned_v<Type> && std::_Is_nonbool_integral<Type>, Return>;

template <typename Type, typename Return = void>
using EnableBool = std::enable_if_t<std::is_integral_v<Type> && !std::_Is_nonbool_integral<Type>, Return>;

template <typename Type, typename Return = void>
using EnableFloat = std::enable_if_t<std::is_floating_point_v<Type>, Return>;

template <typename Type, typename Return = void>
using EnablePointer = std::enable_if_t<std::is_pointer_v<Type>, Return>;

template <typename T0, typename T1>
inline constexpr bool IsSame = std::is_same_v<T0, T1>;

template <typename T0, typename T1>
inline constexpr bool IsSameNoQuals = std::is_same_v<std::remove_cv_t<T0>, std::remove_cv_t<T1>>;