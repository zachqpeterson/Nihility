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

/*---------PLATFORM DETECTION---------*/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _WIN64 //---WINDOWS
#	define NH_PLATFORM_WINDOWS //Defined when on a Windows operating system
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#	ifndef _WIN64
#		error "64-bit is required on Windows!"
#	endif

#elif defined __linux__ || defined __gnu_linux__ //-----------------------------LINUX
#	define NH_PLATFORM_LINUX  //Defined when on a Liunx operating system
#	if defined __ANDROID__ //---------------------------------------------------ANDROID
#		define NH_PLATFORM_ANDROID
#	endif

#elif defined __unix__ //-------------------------------------------------------UNIX
#	define NH_PLATFORM_UNIX  //Defined when on a Unix operating system

#elif defined _POSIX_VERSION_ //------------------------------------------------POSIX
#	define NH_PLATFORM_POSIX

#elif defined __APPLE__ || defined __MACH__ //----------------------------------APPLE
#	define NH_PLATFORM_APPLE  //Defined when on an Apple operating system
#	include <TargetConditionals.h>
#	if TARGET_IPHONE_SIMULATOR //-----------------------------------------------IOS SIMULATOR
#		define NH_PLATFORM_IOS  //Defined when on an IOS operating system
#		define NH_PLATFORM_IOS_SIMULATOR  //Defined when on an IOS Simulator
#	elif TARGET_OS_IPHONE //----------------------------------------------------IOS
#		define NH_PLATFORM_IOS  //Defined when on an IOS operating system
#	elif TARGET_OS_MAC
#	else
#		error "Unknown Apple platform!"
#	endif
#elif defined __EMSCRIPTEN__
#	define NH_PLATFORM_WASM
#else //------------------------------------------------------------------------UNKOWN
#	error "Unknown platform!"
#endif

/*---------ENDIAN DETECTION---------*/

#if defined NH_PLATFORM_WINDOWS || defined __LITTLE_ENDIAN__ || (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#	define NH_LITTLE_ENDIAN  //Defined when on a little endian operating system
#elif defined __BIG_ENDIAN__ || (defined __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#	define NH_BIG_ENDIAN //Defined when on a big endian operating system
#else
#	warning "could not determine endianness! Falling back to little endian..."
#	define NH_LITTLE_ENDIAN //Defined when on a little endian operating system
#endif

/*---------CPU DETECTION---------*/

#if defined( __x86_64__ ) || defined( _M_X64 ) || defined( __i386__ ) || defined( _M_IX86 )
#	define NH_CPU_X86_X64
#elif defined( __aarch64__ ) || defined( _M_ARM64 ) || defined( __arm__ ) || defined( _M_ARM )
#	define NH_CPU_ARM
#elif defined( __EMSCRIPTEN__ )
#	define NH_CPU_WASM
#else
#	define NH_CPU_UNKNOWN
#endif

/*---------SIMD DETECTION---------*/

#ifdef NH_CPU_X86_X64
#	ifdef __AVX__
#		define NH_SIMD_AVX
#		define NH_SIMD_WIDTH 8
#	elif (defined _M_AMD64 || defined _M_X64) || _M_IX86_FP == 2
#		define NH_SIMD_SSE2
#		define NH_SIMD_WIDTH 4
#	elif _M_IX86_FP == 1
#		define NH_SIMD_SSE
#		define NH_SIMD_WIDTH 4
#	endif
#elif defined NH_CPU_ARM
#	define NH_SIMD_NEON
#	define NH_SIMD_WIDTH 4
#elif defined NH_CPU_WASM
#	define NH_SIMD_SSE2
#	define NH_SIMD_WIDTH 4
#else
#	define NH_SIMD_NONE
#	define NH_SIMD_WIDTH 4
#endif

/*---------COMPILER DETECTION---------*/

#if defined __clang__
#	define NH_COMPILER_CLANG
#elif defined __GNUC__ || defined __gcc__
#	define NH_COMPILER_GCC
#elif defined _MSC_VER
#	define NH_COMPILER_MSVC
#endif

/*---------PLATFORM MACROS---------*/

#ifdef _DEBUG
#	define NH_DEBUG				// Defined if running in debug mode
#	define ASSERTIONS_ENABLED	// Defined if assertions are to be enabled
#else
#	define NH_RELEASE			// Defined if running in release mode
#endif

#ifdef NH_EXPORT
#	ifdef NH_COMPILER_MSVC
#		define NH_API __declspec(dllexport)						// Marks a function or class to be exported
#	else
#		define NH_API __attribute__((visibility("default")))	// Marks a function or class to be exported
#	endif
#else
#	ifdef NH_COMPILER_MSVC
#		define NH_API __declspec(dllimport)						// Marks a function or class to be imported
#	else
#		define NH_API											// Marks a function or class to be imported
#	endif
#endif

#define FUNCTION_NAME __FUNCTION__	// Replaced by the name of this function ex. Class::Function
#define FILE_NAME __FILE__			// Replaced by the name of this file ex. C:/file.cpp
#define LINE_NUMBER __LINE__		// Replaced by the line number ex. 123

/// <summary>
/// Deletes a class's constructors, assignment operators, and destructor
/// </summary>
/// <param name="class:">The class to operate on</param>
#define STATIC_CLASS(class)			\
class() = delete;					\
~class() = delete;					\
class(class&) = delete;				\
class(class&&) = delete;			\
class& operator=(class&) = delete;	\
class& operator=(class&&) = delete;	\

#if defined NH_COMPILER_CLANG || defined NH_COMPILER_GCC
#	define NH_INLINE __attribute__((always_inline)) inline	// Tries to force the compiler to inline a function
#	define NH_NOINLINE __attribute__((noinline))			// Tries to force the compiler to not inline a function
#elif defined NH_COMPILER_MSVC
#	define NH_INLINE __forceinline							// Tries to force the compiler to inline a function
#	define NH_NOINLINE __declspec(noinline)					// Tries to force the compiler to not inline a function
#else
#	define NH_INLINE										// Tries to force the compiler to inline a function  (NOT AVAILABLE)
#	define NH_NOINLINE										// Tries to force the compiler to not inline a function (NOT AVAILABLE)
#endif

#ifndef __has_cpp_attribute
#	define HAS_NODISCARD 0
#elif __has_cpp_attribute(nodiscard) >= 201603L
#	define HAS_NODISCARD 1
#else
#	define HAS_NODISCARD 0
#endif

#if HAS_NODISCARD
#	define NH_NODISCARD [[nodiscard]] // Issues a warning when the return value of a function isn't captured
#	define NH_NODISCARD_MSG(message) [[nodiscard(message)]] // Issues a warning when the return value of a function isn't captured
#else
#	define NH_NODISCARD // Issues a warning when the return value of a function isn't captured (NOT AVAILABLE)
#	define NH_NODISCARD_MSG(message) // Issues a warning when the return value of a function isn't captured (NOT AVAILABLE)
#endif

extern "C" { extern int __isa_available; }

enum ISAAvailability
{
	ISA_AVAILABLE_X86 = 0,
	ISA_AVAILABLE_SSE2 = 1,
	ISA_AVAILABLE_SSE42 = 2,
	ISA_AVAILABLE_AVX = 3,
	ISA_AVAILABLE_ENFSTRG = 4,
	ISA_AVAILABLE_AVX2 = 5,
	ISA_AVAILABLE_AVX512 = 6,

	ISA_AVAILABLE_ARMNT = 0,
	ISA_AVAILABLE_NEON = 1,
	ISA_AVAILABLE_NEON_ARM64 = 2,
};

/*---------ASSERTIONS---------*/

#include <intrin.h>

#ifdef ASSERTIONS_ENABLED
#	ifdef NH_COMPILER_MSVC
#		define BreakPoint __debugbreak()	// Halts the execution of the program when reached
#	elif defined NH_COMPILER_GCC || defined NH_COMPILER_CLANG
#		define BreakPoint __builtin_trap()	// Halts the execution of the program when reached
#	else
#		include <assert.h>
#		define BreakPoint assert(0)
#	endif

	/// <summary>
	/// Halts the execution of the program if expr is false
	/// </summary>
	/// <param name="expr:">The expression to check</param>
#	define ASSERT(expr) if (!(expr)) { BreakPoint; }

#else
#	define BreakPoint
#	define ASSERT(expr) expr;
#endif

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

/// <summary>
/// Gets the next multiple of n greater than or equal to value
/// </summary>
/// <param name="value:">Initial value</param>
/// <param name="n:">Multiple</param>
/// <returns>The next multaple of n</returns>
constexpr U64 NextMultipleOf(U64 value, U64 n)
{
	if (n == 0) { return value; }

	U64 remainder = value % n;
	if (remainder == 0) { return value; }

	return value + n - remainder;
}

/// <summary>
/// Bit casts from one type to another, From and To must have the same size
/// </summary>
/// <param name="value:">The value to be casted</param>
/// <returns>The casted value</returns>
template<class To, class From> requires (sizeof(From) == sizeof(To))
NH_NODISCARD constexpr To TypePun(const From& value) noexcept
{
	return __builtin_bit_cast(To, value);
}