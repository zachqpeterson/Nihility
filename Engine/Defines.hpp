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

/*---------MEMORY---------*/

/// <summary>
/// Sets value to the next multiple of 2^alignment
/// </summary>
/// <param name="value:">The value to set</param>
/// <param name="alignment:">The power of 2 to align to</param>
#define AlignPow2(value, alignment) ((value + (alignment - 1)) & ~(alignment - 1))

/// <summary>
/// Sets value to the next multiple of 2
/// </summary>
/// <param name="value:">The value to set</param>
#define Align2(value) ((value + 1) & ~1)

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

/*---------MOVE---------*/

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

/*---------TYPE TRAITS---------*/

#pragma region TypeTraits

template <class Type, Type Value>
struct TypeConstant {
	static constexpr Type value = Value;

	using valueType = Type;
	using type = TypeConstant;

	constexpr operator valueType() const noexcept { return value; }
	NH_NODISCARD constexpr valueType operator()() const noexcept { return value; }
};

template <bool Value>
using BoolConstant = TypeConstant<bool, Value>;

using TrueConstant = BoolConstant<true>;
using FalseConstant = BoolConstant<false>;

template <class, class> inline constexpr bool IsSame = false;
template <class Type> inline constexpr bool IsSame<Type, Type> = true;

namespace TypeTraits
{
	template <class Type> struct RemoveConst { using type = Type; };
	template <class Type> struct RemoveConst<const Type> { using type = Type; };

	template <class Type> struct RemoveVolatile { using type = Type; };
	template <class Type> struct RemoveVolatile<volatile Type> { using type = Type; };

	template <class Type> struct RemoveQuals { using type = Type; };
	template <class Type> struct RemoveQuals<const Type> { using type = Type; };
	template <class Type> struct RemoveQuals<volatile Type> { using type = Type; };
	template <class Type> struct RemoveQuals<const volatile Type> { using type = Type; };

	template <class Type> struct RemoveReference { using type = Type; };
	template <class Type> struct RemoveReference<Type&> { using type = Type; };
	template <class Type> struct RemoveReference<Type&&> { using type = Type; };

	template <class Type> struct RemovePointer { using type = Type; };
	template <class Type> struct RemovePointer<Type*> { using type = Type; };

	template <class Type> struct RemovePointerAll { using type = Type; };
	template <class Type> struct RemovePointerAll<Type*> : public RemovePointerAll<Type> { };

	template <class Type> struct RemoveArray { using type = Type; };
	template <class Type> struct RemoveArray<Type[]> { using type = Type; };
	template <class Type, unsigned long long N> struct RemoveArray<Type[N]> { using type = Type; };

	template <class Type> struct RemoveArrayAll { using type = Type; };
	template <class Type> struct RemoveArrayAll<Type[]> : public RemoveArrayAll<Type> { };
	template <class Type, unsigned long long N> struct RemoveArrayAll<Type[N]> : public RemoveArrayAll<Type> { };

	template <class> struct GetUnsigned { using type = U32; };
	template <> struct GetUnsigned<I8> { using type = U8; };
	template <> struct GetUnsigned<U8> { using type = U8; };
	template <> struct GetUnsigned<I16> { using type = U16; };
	template <> struct GetUnsigned<U16> { using type = U16; };
	template <> struct GetUnsigned<I32> { using type = U32; };
	template <> struct GetUnsigned<U32> { using type = U32; };
	template <> struct GetUnsigned<L32> { using type = UL32; };
	template <> struct GetUnsigned<UL32> { using type = UL32; };
	template <> struct GetUnsigned<I64> { using type = U64; };
	template <> struct GetUnsigned<U64> { using type = U64; };

	template <class> struct GetSigned { using type = I32; };
	template <> struct GetSigned<I8> { using type = I8; };
	template <> struct GetSigned<U8> { using type = I8; };
	template <> struct GetSigned<I16> { using type = I16; };
	template <> struct GetSigned<U16> { using type = I16; };
	template <> struct GetSigned<I32> { using type = I32; };
	template <> struct GetSigned<U32> { using type = I32; };
	template <> struct GetSigned<L32> { using type = L32; };
	template <> struct GetSigned<UL32> { using type = L32; };
	template <> struct GetSigned<I64> { using type = I64; };
	template <> struct GetSigned<U64> { using type = I64; };

	template <bool Test, class Type0, class Type1> struct ConditionalOf { using type = Type0; };
	template <class Type0, class Type1> struct ConditionalOf<false, Type0, Type1> { using type = Type1; };
}

template <bool Test, class Type0, class Type1>
using Conditional = typename TypeTraits::ConditionalOf<Test, Type0, Type1>::type;

namespace TypeTraits
{
	template<typename Type, typename... Rest>
	struct IsAnyOf : FalseConstant {};

	template<typename Type, typename First, typename... Rest>
	struct IsAnyOf<Type, First, Rest...> : Conditional<IsSame<Type, First>, TrueConstant, IsAnyOf<Type, Rest...>> {};
}
	
template<typename Type, typename... Rest>
inline constexpr bool AnyOf = TypeTraits::IsAnyOf<Type, Rest...>::value;

template <class Type> using RemovedConst = typename TypeTraits::RemoveConst<Type>::type;
template <class Type> using RemovedVolatile = typename TypeTraits::RemoveVolatile<Type>::type;
template <class Type> using RemovedQuals = typename TypeTraits::RemoveQuals<Type>::type;
template <class Type> using RemovedReference = typename TypeTraits::RemoveReference<Type>::type;
template <class Type> using RemovedPointer = typename TypeTraits::RemovePointer<Type>::type;
template <class Type> using RemovedPointers = typename TypeTraits::RemovePointerAll<Type>::type;
template <class Type> using RemovedArray = typename TypeTraits::RemoveArray<Type>::type;
template <class Type> using RemovedArrays = typename TypeTraits::RemoveArrayAll<Type>::type;
template <class Type> using BaseType = typename TypeTraits::RemoveQuals<RemovedPointers<RemovedArrays<RemovedReference<Type>>>>::type;
template <class Type> using UnsignedOf = typename TypeTraits::GetUnsigned<Type>::type;
template <class Type> using SignedOf = typename TypeTraits::GetSigned<Type>::type;

template <class> inline constexpr bool IsPointer = false;
template <class Type> inline constexpr bool IsPointer<Type*> = true;
template <class Type> inline constexpr bool IsPointer<const Type*> = true;
template <class Type> inline constexpr bool IsPointer<volatile Type*> = true;
template <class Type> inline constexpr bool IsPointer<const volatile Type*> = true;
template <class Type> concept Pointer = IsPointer<Type>;

template <class Type> inline constexpr bool IsSinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;
template <class Type> concept SinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;

template <class> inline constexpr bool IsLReference = false;
template <class Type> inline constexpr bool IsLReference<Type&> = true;
template <class Type> inline constexpr bool IsLReference<const Type&> = true;
template <class Type> inline constexpr bool IsLReference<volatile Type&> = true;
template <class Type> inline constexpr bool IsLReference<const volatile Type&> = true;
template <class Type> concept LReference = IsLReference<Type>;

template <class> inline constexpr bool IsRReference = false;
template <class Type> inline constexpr bool IsRReference<Type&&> = true;
template <class Type> inline constexpr bool IsRReference<const Type&&> = true;
template <class Type> inline constexpr bool IsRReference<volatile Type&&> = true;
template <class Type> inline constexpr bool IsRReference<const volatile Type&&> = true;
template <class Type> concept RReference = IsRReference<Type>;

template <class Type> inline constexpr bool IsReference = IsLReference<Type> || IsRReference<Type>;
template <class Type> concept Reference = IsLReference<Type> || IsRReference<Type>;

template <class> inline constexpr bool IsArray = false;
template <class Type> inline constexpr bool IsArray<Type[]> = true;
template <class Type> inline constexpr bool IsArray<const Type[]> = true;
template <class Type> inline constexpr bool IsArray<volatile Type[]> = true;
template <class Type> inline constexpr bool IsArray<const volatile Type[]> = true;
template <class Type, unsigned long long N> inline constexpr bool IsArray<Type[N]> = true;
template <class Type, unsigned long long N> inline constexpr bool IsArray<const Type[N]> = true;
template <class Type, unsigned long long N> inline constexpr bool IsArray<volatile Type[N]> = true;
template <class Type, unsigned long long N> inline constexpr bool IsArray<const volatile Type[N]> = true;
template <class Type> concept Array = IsArray<Type>;

template <class Type> inline constexpr bool IsSingleArray = IsArray<Type> && !IsArray<RemovedArray<Type>>;
template <class Type> concept SingleArray = IsArray<Type> && !IsArray<RemovedArray<Type>>;

template <class Type> inline constexpr bool IsCharacter = AnyOf<RemovedQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;
template <class Type> concept Character = AnyOf<RemovedQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;

template <class Type> inline constexpr bool IsBoolean = AnyOf<RemovedQuals<Type>, bool>;
template <class Type> concept Boolean = AnyOf<RemovedQuals<Type>, bool>;

template <class Type> inline constexpr bool IsInteger = AnyOf<RemovedQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int,
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;
template <class Type> concept Integer = AnyOf<RemovedQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int,
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;

template <class Type> inline constexpr bool IsSigned = AnyOf<RemovedQuals<Type>, signed char, short, signed short, int, signed int, long, signed long, long long, signed long long>;
template <class Type> concept Signed = AnyOf<RemovedQuals<Type>, signed char, short, signed short, int, signed int, long, signed long, long long, signed long long>;

template <class Type> inline constexpr bool IsUnsigned = AnyOf<RemovedQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
template <class Type> concept Unsigned = AnyOf<RemovedQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class Type> inline constexpr bool IsFloatingPoint = AnyOf<RemovedQuals<Type>, float, double, long double>;
template <class Type> concept FloatingPoint = AnyOf<RemovedQuals<Type>, float, double, long double>;

template <class Type> inline constexpr bool IsNumber = IsInteger<Type> || IsFloatingPoint<Type>;
template <class Type> concept Number = IsInteger<Type> || IsFloatingPoint<Type>;

template <class Type> inline constexpr bool IsStringLiteral = AnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);
template <class Type> concept StringLiteral = AnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);

template <class Type> struct Traits
{
	using Base = RemovedQuals<Type>;

private:
	static inline constexpr Base GetMaxValue()
	{
		if constexpr (IsSame<Base, char>) { return U8_MAX; }
		if constexpr (IsSame<Base, unsigned char>) { return U8_MAX; }
		if constexpr (IsSame<Base, signed char>) { return I8_MAX; }
		if constexpr (IsSame<Base, char8_t>) { return U8_MAX; }

#ifdef PLATFORM_WINDOWS
		if constexpr (IsSame<Base, wchar_t>) { return U16_MAX; }
#else
		if constexpr (IsSame<Base, wchar_t>) { return U32_MAX; }
#endif
		if constexpr (IsSame<Base, char16_t>) { return U16_MAX; }
		if constexpr (IsSame<Base, short>) { return I16_MAX; }
		if constexpr (IsSame<Base, unsigned short>) { return U16_MAX; }
		if constexpr (IsSame<Base, signed short>) { return I16_MAX; }

		if constexpr (IsSame<Base, char32_t>) { return U32_MAX; }
		if constexpr (IsSame<Base, int>) { return I32_MAX; }
		if constexpr (IsSame<Base, unsigned int>) { return U32_MAX; }
		if constexpr (IsSame<Base, signed int>) { return I32_MAX; }
		if constexpr (IsSame<Base, long>) { return I32_MAX; }
		if constexpr (IsSame<Base, unsigned long>) { return U32_MAX; }
		if constexpr (IsSame<Base, signed long>) { return I32_MAX; }

		if constexpr (IsSame<Base, long long>) { return I64_MAX; }
		if constexpr (IsSame<Base, unsigned long long>) { return U64_MAX; }
		if constexpr (IsSame<Base, signed long long>) { return I64_MAX; }

		if constexpr (IsSame<Base, float>) { return F32_MAX; }
		if constexpr (IsSame<Base, double>) { return F64_MAX; }
	}

	static inline constexpr Base GetMinValue()
	{
		if constexpr (IsSame<Base, char>) { return U8_MIN; }
		if constexpr (IsSame<Base, unsigned char>) { return U8_MIN; }
		if constexpr (IsSame<Base, signed char>) { return I8_MIN; }
		if constexpr (IsSame<Base, char8_t>) { return U8_MIN; }

#ifdef PLATFORM_WINDOWS
		if constexpr (IsSame<Base, wchar_t>) { return U16_MIN; }
#else
		if constexpr (IsSame<Base, wchar_t>) { return U32_MIN; }
#endif
		if constexpr (IsSame<Base, char16_t>) { return U16_MIN; }
		if constexpr (IsSame<Base, short>) { return I16_MIN; }
		if constexpr (IsSame<Base, unsigned short>) { return U16_MIN; }
		if constexpr (IsSame<Base, signed short>) { return I16_MIN; }

		if constexpr (IsSame<Base, char32_t>) { return U32_MIN; }
		if constexpr (IsSame<Base, int>) { return I32_MIN; }
		if constexpr (IsSame<Base, unsigned int>) { return U32_MIN; }
		if constexpr (IsSame<Base, signed int>) { return I32_MIN; }
		if constexpr (IsSame<Base, long>) { return I32_MIN; }
		if constexpr (IsSame<Base, unsigned long>) { return U32_MIN; }
		if constexpr (IsSame<Base, signed long>) { return I32_MIN; }

		if constexpr (IsSame<Base, long long>) { return I64_MIN; }
		if constexpr (IsSame<Base, unsigned long long>) { return U64_MIN; }
		if constexpr (IsSame<Base, signed long long>) { return I64_MIN; }

		if constexpr (IsSame<Base, float>) { return F32_MIN; }
		if constexpr (IsSame<Base, double>) { return F64_MIN; }
	}

public:
	static constexpr Base MaxValue = GetMaxValue();
	static constexpr Base MinValue = GetMinValue();
};

#pragma endregion