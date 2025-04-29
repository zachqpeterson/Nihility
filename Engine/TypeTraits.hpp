#pragma once

#include "Defines.hpp"

#include <utility>
#include <type_traits>
#include <bit>

template <class Derived, class Base> constexpr const bool InheritsFrom = __is_base_of(Base, Derived) && __is_convertible_to(const volatile Derived*, const volatile Base*);

template <class Type> constexpr const bool IsClass = __is_class(Type);
template <class Type> concept Class = IsClass<Type>;

template <class Type> constexpr const bool IsUnion = __is_union(Type);
template <class Type> concept Union = IsUnion<Type>;

template <class Type> constexpr const bool IsNonPrimitive = IsClass<Type> || IsUnion<Type>;
template <class Type> concept NonPrimitive = IsNonPrimitive<Type>;

template <class Type> constexpr const bool IsNothrowMoveConstructible = __is_nothrow_constructible(Type, Type);
template <class Type> concept NothrowMoveConstructible = IsNothrowMoveConstructible<Type>;

template <class Type> constexpr const unsigned long long Alignment = alignof(Type);

using TrueConstant = std::bool_constant<true>;
using FalseConstant = std::bool_constant<false>;

[[nodiscard]] constexpr bool ConstantEvaluation() noexcept
{
	return __builtin_is_constant_evaluated();
}

template<bool Condition, class TrueResult, class FalseResult>
using Conditional = std::conditional_t<Condition, TrueResult, FalseResult>;

namespace TypeTraits
{
	template <class Type> struct RemovePointerAll { using type = Type; };
	template <class Type> struct RemovePointerAll<Type*> : public RemovePointerAll<Type> {};
	template <class Type, unsigned long long Count> struct GetPointerCount { static constexpr unsigned long long count = Count; };
	template <class Type, unsigned long long Count> struct GetPointerCount<Type*, Count> : public GetPointerCount<Type, Count + 1> {};
	template<class Type> struct AppliedPointers { using type = Type; };
	template<class Type, unsigned long long Count> struct ApplyPointers : Conditional<Count == 0, AppliedPointers<Type>, ApplyPointers<Type*, Count - 1>> {};

	template<class Type>
	struct IsDestroyable
	{
		template<class C> static constexpr TrueConstant Test(decltype(&C::Destroy));
		template<class> static constexpr FalseConstant Test(...);

		static constexpr inline bool value = decltype(Test<Type>(0))::value;
	};

	template<class From, class To>
	struct IsConvertableTo //TODO: Same as __is_convertible_to ?
	{
		template<class C> static constexpr TrueConstant Test(decltype(&C::operator To));
		template<class> static constexpr FalseConstant Test(...);

		static constexpr inline bool value = decltype(Test<From>(0))::value;
	};

	template <class Type, Type... Values>
	struct Sequence
	{
		using ValueType = Type;

		[[nodiscard]] static constexpr unsigned long long Size() noexcept
		{
			return sizeof...(Values);
		}
	};
}

template <unsigned long long... Values> using IndexSequence = TypeTraits::Sequence<unsigned long long, Values...>;

template <class Type, Type Size> using MakeSequence = __make_integer_seq<TypeTraits::Sequence, Type, Size>;
template <unsigned long long Size> using MakeIndexSequence = __make_integer_seq<TypeTraits::Sequence, unsigned long long, Size>;

template <class> constexpr const bool True = false;
template <class> constexpr const bool False = false;

template <class... Types> constexpr bool Conjunction = std::conjunction_v<Types...>;

template <class Type> using RemoveConst = std::remove_const_t<Type>;
template <class Type> using RemoveVolatile = std::remove_volatile_t<Type>;
template <class Type> using RemoveQuals = std::remove_cv_t<Type>;
template <class Type> using RemoveReference = std::remove_reference_t<Type>;
template <class Type> using RemoveQualsReference = std::remove_cvref_t<Type>;
template <class Type> using RemovePointer = std::remove_pointer_t<Type>;
template <class Type> using AddPointer = std::add_pointer_t<Type>;
template <class Type> using RemovePointers = TypeTraits::RemovePointerAll<Type>::type;
template <class Type> using RemoveArray = std::remove_extent_t<Type>;
template <class Type> using RemoveArrays = std::remove_all_extents_t<Type>;
template <class Type> using AddLvalReference = std::add_lvalue_reference_t<Type>;
template <class Type> using AddRvalReference = std::add_rvalue_reference_t<Type>;
template <class Type> using BaseType = RemoveQualsReference<RemovePointers<RemoveArrays<Type>>>;
template <class Type> using Decayed = std::decay_t<Type>;
template <class Type> using UnsignedOf = std::make_unsigned_t<Type>;
template <class Type> using SignedOf = std::make_signed_t<Type>;

template <class Type> constexpr const unsigned long long PointerCount = TypeTraits::GetPointerCount<Type, 0>::count;
template <class Type, unsigned long long Count> using AddPointers = TypeTraits::ApplyPointers<Type, Count>::type;

template <class Type> constexpr const bool IsPointer = std::is_pointer_v<Type>;
template <class Type> concept Pointer = IsPointer<Type>;
template <class Type> concept NonPointer = !IsPointer<Type>;

template <class Type> constexpr const bool IsSinglePointer = IsPointer<Type> && !IsPointer<RemovePointer<Type>>;
template <class Type> concept SinglePointer = IsSinglePointer<Type>;

template <class Type> constexpr const bool IsLReference = std::is_lvalue_reference_v<Type>;
template <class Type> concept LvalReference = IsLReference<Type>;

template <class Type> constexpr const bool IsRReference = std::is_rvalue_reference_v<Type>;
template <class Type> concept RvalReference = IsRReference<Type>;

template <class Type> constexpr const bool IsReference = IsLReference<Type> || IsRReference<Type>;
template <class Type> concept Reference = IsReference<Type>;

template <class Type> constexpr const bool IsArray = std::is_array_v<Type>;

template <class Type> constexpr const bool IsSingleArray = IsArray<Type> && !IsArray<RemoveArray<Type>>;
template <class Type> concept SingleArray = IsSingleArray<Type>;

template <bool Test, class Type = void> using Enable = std::enable_if_t<Test, Type>;
template<class Type, class... Rest> constexpr const bool AnyOf = std::_Is_any_of_v<Type, Rest...>;

template <class Type> constexpr const bool IsCharacter = AnyOf<RemoveQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;
template <class Type> concept Character = IsCharacter<Type>;

template <class Type> constexpr const bool IsBoolean = AnyOf<RemoveQuals<Type>, bool>;
template <class Type> concept Boolean = IsBoolean<Type>;

template <class Type> constexpr const bool IsInteger = AnyOf<RemoveQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int,
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;
template <class Type> concept Integer = IsInteger<Type>;

template <class Type> constexpr const bool IsSigned = AnyOf<RemoveQuals<Type>, signed char, short, signed short, int, signed int, long, signed long, long long, signed long long>;
template <class Type> concept Signed = IsSigned<Type>;

template <class Type> constexpr const bool IsUnsigned = AnyOf<RemoveQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
template <class Type> concept Unsigned = IsUnsigned<Type>;

template <class Type> constexpr const bool IsFloatingPoint = AnyOf<RemoveQuals<Type>, float, double, long double>;
template <class Type> concept FloatingPoint = IsFloatingPoint<Type>;

template <class Type> constexpr const bool IsNumber = IsInteger<Type> || IsFloatingPoint<Type>;
template <class Type> concept Number = IsNumber<Type>;

template <class Type> constexpr const bool IsEnum = std::is_enum_v<Type>;
template <class Type> concept Enum = IsEnum<Type>;

template <class Type> constexpr const bool IsStringLiteral = AnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);
template <class Type> concept StringLiteral = IsStringLiteral<Type>;

template <class Type> inline constexpr bool IsNonStringPointer = IsPointer<Type> && !IsStringLiteral<Type>;
template <class Type> concept NonStringPointer = IsNonStringPointer<Type>;

template <class Type> constexpr const bool IsFunctionPtr = std::is_function_v<Type>;
template <class Type> concept FunctionPtr = requires (Type t) { IsFunctionPtr<decltype(t)>; };

template <class Type> constexpr const bool IsMemberFunctionPtr = std::is_member_function_pointer_v<Type>;
template <class Type> concept MemberFunctionPtr = requires (Type t) { IsMemberFunctionPtr<decltype(t)>; };

template <class Type, class... Args> constexpr bool IsInvocable = std::is_invocable_v<Type, Args...>;
template <class Type, class... Args> concept Invocable = IsInvocable<Type, Args...>;

template <class Type> constexpr const bool IsObject = std::is_object_v<Type>;
template <class Type> concept Object = IsObject<Type>;

template <class Type, class... Args> constexpr bool IsConstructible = __is_constructible(Type, Args...);
template <class Type> concept Constructible = IsConstructible<Type>;

template <class Type> constexpr const bool IsCopyConstructible = __is_constructible(Type, AddLvalReference<const Type>);
template <class Type> concept CopyConstructible = IsCopyConstructible<Type>;

template <class Type> constexpr const bool IsDestructible = __is_destructible(Type);
template <class Type> concept Destructible = IsDestructible<Type>;

template <class Type> constexpr const bool IsCopyAssignable = __is_assignable(AddLvalReference<Type>, AddLvalReference<const Type>);
template <class Type> concept CopyAssignable = IsCopyAssignable<Type>;

template <class Type> constexpr const bool IsMoveConstructible = __is_constructible(Type, Type);
template <class Type> concept MoveConstructible = IsMoveConstructible<Type>;

template <class Type> constexpr const bool IsMoveAssignable = __is_assignable(AddLvalReference<Type>, Type);
template <class Type> concept MoveAssignable = IsMoveAssignable<Type>;

template <class Type> constexpr const bool IsCopyable = IsCopyConstructible<Type> || IsCopyAssignable<Type>;
template <class Type> concept Copyable = IsCopyable<Type>;

template <class Type> constexpr const bool IsMoveable = IsMoveConstructible<Type> || IsMoveAssignable<Type>;
template <class Type> concept Moveable = IsMoveable<Type>;

template <class Type> constexpr const bool IsCopyOrMoveable = IsCopyable<Type> || IsMoveable<Type>;
template <class Type> concept CopyOrMoveable = IsCopyOrMoveable<Type>;

template <class, class> constexpr const bool IsSame = false;
template <class Type> constexpr const bool IsSame<Type, Type> = true;

template <class> constexpr const bool IsConst = false;
template <class Type> constexpr const bool IsConst<const Type> = true;

template <class> constexpr const bool IsVolatile = false;
template <class Type> constexpr const bool IsVolatile<volatile Type> = true;

template <class> constexpr const bool IsConstVolatile = false;
template <class Type> constexpr const bool IsConstVolatile<const volatile Type> = true;

template <class Type> constexpr const bool IsDestroyable = TypeTraits::IsDestroyable<Type>::value;
template <class Type> concept Destroyable = IsDestroyable<Type>;

template <class From, class To> constexpr const bool ConvertibleTo = TypeTraits::IsConvertableTo<From, To>::value;

template <class Type> constexpr const bool IsVoid = IsSame<RemoveQuals<Type>, void>;

template <class... Types> using CommonType = std::common_type_t<Types...>;

template<class Type, template<class...> class U> constexpr const bool IsSpecializationOf = false;
template<template<class...> class U, class... Vs> constexpr const bool IsSpecializationOf<U<Vs...>, U> = true;

/// <summary>
/// Forwards arg as movable
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template<class Type> constexpr RemoveReference<Type>&& Move(Type&& arg) noexcept { return static_cast<RemoveReference<Type>&&>(arg); }

/// <summary>
/// Forwards an lValue as an rValue
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template <class Type> constexpr Type&& Forward(RemoveReference<Type>& arg) noexcept { return static_cast<Type&&>(arg); }

/// <summary>
/// Forwards an lValue as an rValue
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template <class Type> constexpr Type&& Forward(RemoveReference<Type>&& arg) noexcept { static_assert(!IsLReference<Type>, "Bad Forward Call"); return static_cast<Type&&>(arg); }

template<class T> AddRvalReference<T> DeclValue() noexcept { static_assert(False<T>, "GetReference not allowed in an evaluated context"); }

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

template<class Type> constexpr void Swap(Type& a, Type& b) noexcept
{
	Type tmp = Move(a);
	a = Move(b);
	b = Move(tmp);
}

namespace TypeTraits
{
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
}

//TODO: long double
template <class Type> struct Traits
{
	using Base = RemoveQuals<Type>;

private:
	static constexpr Base GetMaxValue()
	{
		if constexpr (IsSame<Base, char>) { return TypeTraits::U8_MAX; }
		if constexpr (IsSame<Base, unsigned char>) { return TypeTraits::U8_MAX; }
		if constexpr (IsSame<Base, signed char>) { return TypeTraits::I8_MAX; }
		if constexpr (IsSame<Base, char8_t>) { return TypeTraits::U8_MAX; }

#ifdef NH_PLATFORM_WINDOWS
		if constexpr (IsSame<Base, wchar_t>) { return TypeTraits::U16_MAX; }
#else
		if constexpr (IsSame<Base, wchar_t>) { return TypeTraits::U32_MAX; }
#endif
		if constexpr (IsSame<Base, char16_t>) { return TypeTraits::U16_MAX; }
		if constexpr (IsSame<Base, short>) { return TypeTraits::I16_MAX; }
		if constexpr (IsSame<Base, unsigned short>) { return TypeTraits::U16_MAX; }
		if constexpr (IsSame<Base, signed short>) { return TypeTraits::I16_MAX; }

		if constexpr (IsSame<Base, char32_t>) { return TypeTraits::U32_MAX; }
		if constexpr (IsSame<Base, int>) { return TypeTraits::I32_MAX; }
		if constexpr (IsSame<Base, unsigned int>) { return TypeTraits::U32_MAX; }
		if constexpr (IsSame<Base, signed int>) { return TypeTraits::I32_MAX; }
		if constexpr (IsSame<Base, long>) { return TypeTraits::I32_MAX; }
		if constexpr (IsSame<Base, unsigned long>) { return TypeTraits::U32_MAX; }
		if constexpr (IsSame<Base, signed long>) { return TypeTraits::I32_MAX; }

		if constexpr (IsSame<Base, long long>) { return TypeTraits::I64_MAX; }
		if constexpr (IsSame<Base, unsigned long long>) { return TypeTraits::U64_MAX; }
		if constexpr (IsSame<Base, signed long long>) { return TypeTraits::I64_MAX; }

		if constexpr (IsSame<Base, float>) { return TypeTraits::F32_MAX; }
		if constexpr (IsSame<Base, double>) { return TypeTraits::F64_MAX; }
	}

	static constexpr Base GetMinValue()
	{
		if constexpr (IsSame<Base, char>) { return TypeTraits::U8_MIN; }
		if constexpr (IsSame<Base, unsigned char>) { return TypeTraits::U8_MIN; }
		if constexpr (IsSame<Base, signed char>) { return TypeTraits::I8_MIN; }
		if constexpr (IsSame<Base, char8_t>) { return TypeTraits::U8_MIN; }

#ifdef NH_PLATFORM_WINDOWS
		if constexpr (IsSame<Base, wchar_t>) { return TypeTraits::U16_MIN; }
#else
		if constexpr (IsSame<Base, wchar_t>) { return TypeTraits::U32_MIN; }
#endif
		if constexpr (IsSame<Base, char16_t>) { return TypeTraits::U16_MIN; }
		if constexpr (IsSame<Base, short>) { return TypeTraits::I16_MIN; }
		if constexpr (IsSame<Base, unsigned short>) { return TypeTraits::U16_MIN; }
		if constexpr (IsSame<Base, signed short>) { return TypeTraits::I16_MIN; }

		if constexpr (IsSame<Base, char32_t>) { return TypeTraits::U32_MIN; }
		if constexpr (IsSame<Base, int>) { return TypeTraits::I32_MIN; }
		if constexpr (IsSame<Base, unsigned int>) { return TypeTraits::U32_MIN; }
		if constexpr (IsSame<Base, signed int>) { return TypeTraits::I32_MIN; }
		if constexpr (IsSame<Base, long>) { return TypeTraits::I32_MIN; }
		if constexpr (IsSame<Base, unsigned long>) { return TypeTraits::U32_MIN; }
		if constexpr (IsSame<Base, signed long>) { return TypeTraits::I32_MIN; }

		if constexpr (IsSame<Base, long long>) { return TypeTraits::I64_MIN; }
		if constexpr (IsSame<Base, unsigned long long>) { return TypeTraits::U64_MIN; }
		if constexpr (IsSame<Base, signed long long>) { return TypeTraits::I64_MIN; }

		if constexpr (IsSame<Base, float>) { return TypeTraits::F32_MIN; }
		if constexpr (IsSame<Base, double>) { return TypeTraits::F64_MIN; }
	}

	static constexpr unsigned long long GetNumericalBits()
	{
		if constexpr (IsSame<Base, char>) { return 8; }
		if constexpr (IsSame<Base, unsigned char>) { return 8; }
		if constexpr (IsSame<Base, signed char>) { return 7; }
		if constexpr (IsSame<Base, char8_t>) { return 8; }

#ifdef NH_PLATFORM_WINDOWS
		if constexpr (IsSame<Base, wchar_t>) { return 16; }
#else
		if constexpr (IsSame<Base, wchar_t>) { return 32; }
#endif
		if constexpr (IsSame<Base, char16_t>) { return 16; }
		if constexpr (IsSame<Base, short>) { return 15; }
		if constexpr (IsSame<Base, unsigned short>) { return 16; }
		if constexpr (IsSame<Base, signed short>) { return 15; }

		if constexpr (IsSame<Base, char32_t>) { return 32; }
		if constexpr (IsSame<Base, int>) { return 31; }
		if constexpr (IsSame<Base, unsigned int>) { return 32; }
		if constexpr (IsSame<Base, signed int>) { return 31; }
		if constexpr (IsSame<Base, long>) { return 31; }
		if constexpr (IsSame<Base, unsigned long>) { return 32; }
		if constexpr (IsSame<Base, signed long>) { return 31; }

		if constexpr (IsSame<Base, long long>) { return 63; }
		if constexpr (IsSame<Base, unsigned long long>) { return 64; }
		if constexpr (IsSame<Base, signed long long>) { return 63; }

		if constexpr (IsSame<Base, float>) { return 24; }
		if constexpr (IsSame<Base, double>) { return 53; }

		if constexpr (IsSame<Base, bool>) { return 1; }
	}

	static constexpr Base GetInfinity()
	{
		if constexpr (IsSame<Base, float>) { return __builtin_huge_valf(); }
		if constexpr (IsSame<Base, double>) { return __builtin_huge_val(); }

		return 0;
	}

	static constexpr Base GetNaN()
	{
		if constexpr (IsSame<Base, float>) { return __builtin_nanf("0"); }
		if constexpr (IsSame<Base, double>) { return __builtin_nan("0"); }

		return 0;
	}

	static constexpr Base GetEpsilon()
	{
		if constexpr (IsSame<Base, float>) { return 1.192092896e-06F; }
		if constexpr (IsSame<Base, double>) { return 2.22045e-16; }

		return 0;
	}

	static constexpr Base GetMaxPrecision()
	{
		if constexpr (IsSame<Base, float>) { return (float)(1i64 << 23); }
		if constexpr (IsSame<Base, double>) { return (double)(1i64 << 52); }

		return 0;
	}

public:
	static constexpr inline Base MaxValue = GetMaxValue();
	static constexpr inline Base MinValue = GetMinValue();
	static constexpr inline unsigned long long NumericalBits = GetNumericalBits();
	static constexpr inline Base Infinity = GetInfinity();
	static constexpr inline Base NaN = GetNaN();
	static constexpr inline Base Epsilon = GetEpsilon();
	static constexpr inline Base MaxPrecision = GetMaxPrecision();
};

template<Unsigned T>
[[nodiscard]] constexpr T BitCeiling(const T val) noexcept
{
	if (val <= 1u) { return T{ 1 }; }

	return static_cast<T>(T{ 1 } << (Traits<T>::NumericalBits - std::countl_zero(static_cast<T>(val - 1))));
}

template<Unsigned T>
[[nodiscard]] constexpr T BitFloor(const T val) noexcept
{
	if (val == 0u) { return val; }

	return static_cast<T>(T{ 1 } << (Traits<T>::NumericalBits - 1 - std::countl_zero(val)));
}

template<Unsigned T>
[[nodiscard]] constexpr T DegreeOfTwo(const T val) noexcept
{
	if (val <= 1u) { return T{ 0 }; }

	return static_cast<T>(Traits<T>::NumericalBits - 1 - std::countl_zero(val));
}

template<class Type>
[[nodiscard]] constexpr std::underlying_type_t<Type> operator*(Type value) noexcept
{
	static_assert(std::is_enum_v<Type>, "Type must be an enum");

	return static_cast<std::underlying_type_t<Type>>(value);
}