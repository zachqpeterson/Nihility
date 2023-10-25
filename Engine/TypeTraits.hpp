#pragma once

template <class Derived, class Base> inline constexpr bool InheritsFrom = __is_base_of(Base, Derived) && __is_convertible_to(const volatile Derived*, const volatile Base*);

template <class Type> inline constexpr bool IsClass = __is_class(Type);
template <class Type> concept Class = IsClass<Type>;

template <class Type> inline constexpr bool IsUnion = __is_union(Type);
template <class Type> concept Union = IsUnion<Type>;

template <class Type> inline constexpr bool IsNothrowMoveConstructible = __is_nothrow_constructible(Type, Type);
template <class Type> concept NothrowMoveConstructible = IsNothrowMoveConstructible<Type>;

template <class Type> inline constexpr unsigned long long Alignment = alignof(Type);

NH_NODISCARD constexpr bool ConstantEvaluation() noexcept
{
	return __builtin_is_constant_evaluated();
}

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

template <class> inline constexpr bool True = false;
template <class> inline constexpr bool False = false;

template <class... Types> using Void = void;

template <class, class> inline constexpr bool IsSame = false;
template <class Type> inline constexpr bool IsSame<Type, Type> = true;

template <class> inline constexpr bool IsConst = false;
template <class Type> inline constexpr bool IsConst<const Type> = true;

template <class> inline constexpr bool IsVolatile = false;
template <class Type> inline constexpr bool IsVolatile<volatile Type> = true;

template <class> inline constexpr bool IsConstVolatile = false;
template <class Type> inline constexpr bool IsConstVolatile<const volatile Type> = true;

namespace TypeTraits
{
	template <class Type> struct RemoveConst { using type = Type; };
	template <class Type> struct RemoveConst<const Type> { using type = Type; };

	template <class Type> struct RemoveVolatile { using type = Type; };
	template <class Type> struct RemoveVolatile<volatile Type> { using type = Type; };

	template <class Type> struct RemoveQuals { using type = Type; template <template <class> class Fn> using Apply = Fn<Type>; };
	template <class Type> struct RemoveQuals<const Type> { using type = Type; template <template <class> class Fn> using Apply = const Fn<Type>; };
	template <class Type> struct RemoveQuals<volatile Type> { using type = Type; template <template <class> class Fn> using Apply = volatile Fn<Type>; };
	template <class Type> struct RemoveQuals<const volatile Type> { using type = Type; template <template <class> class Fn> using Apply = const volatile Fn<Type>; };

	template <class Type> struct RemoveReference { using type = Type; };
	template <class Type> struct RemoveReference<Type&> { using type = Type; };
	template <class Type> struct RemoveReference<Type&&> { using type = Type; };

	template <class Type> struct RemovePointer { using type = Type; };
	template <class Type> struct RemovePointer<Type*> { using type = Type; };

	template <class Type, class = void> struct AddPointer { using type = Type; };
	template <class Type> struct AddPointer<Type, Void<typename RemoveReference<Type>::type*>> { using type = typename RemoveReference<Type>::type*; };

	template <class Type> struct RemovePointerAll { using type = Type; };
	template <class Type> struct RemovePointerAll<Type*> : public RemovePointerAll<Type> { };

	template <class Type, U64 Count> struct GetPointerCount { static constexpr U64 count = Count; };
	template <class Type, U64 Count> struct GetPointerCount<Type*, Count> : public GetPointerCount<Type, Count + 1> { };

	template <class Type> struct RemoveArray { using type = Type; };
	template <class Type> struct RemoveArray<Type[]> { using type = Type; };
	template <class Type, unsigned long long N> struct RemoveArray<Type[N]> { using type = Type; };

	template <class Type> struct RemoveArrayAll { using type = Type; };
	template <class Type> struct RemoveArrayAll<Type[]> : public RemoveArrayAll<Type> { };
	template <class Type, unsigned long long N> struct RemoveArrayAll<Type[N]> : public RemoveArrayAll<Type> { };

	template <class Type, class = void> struct AddReference { using lValue = Type; using rValue = Type; };
	template <class Type> struct AddReference<Type, Void<Type&>> { using lValue = Type&; using rValue = Type&&; };

	template <class> struct GetUnsigned { using type = unsigned int; };
	template <> struct GetUnsigned<char> { using type = unsigned char; };
	template <> struct GetUnsigned<signed char> { using type = unsigned char; };
	template <> struct GetUnsigned<unsigned char> { using type = unsigned char; };
	template <> struct GetUnsigned<short> { using type = unsigned short; };
	template <> struct GetUnsigned<unsigned short> { using type = unsigned short; };
	template <> struct GetUnsigned<int> { using type = unsigned int; };
	template <> struct GetUnsigned<unsigned int> { using type = unsigned int; };
	template <> struct GetUnsigned<long> { using type = unsigned long; };
	template <> struct GetUnsigned<unsigned long> { using type = unsigned long; };
	template <> struct GetUnsigned<long long> { using type = unsigned long long; };
	template <> struct GetUnsigned<unsigned long long> { using type = unsigned long long; };

	template <class> struct GetSigned { using type = signed int; };
	template <> struct GetSigned<char> { using type = signed char; };
	template <> struct GetSigned<signed char> { using type = signed char; };
	template <> struct GetSigned<unsigned char> { using type = signed char; };
	template <> struct GetSigned<short> { using type = signed short; };
	template <> struct GetSigned<unsigned short> { using type = signed short; };
	template <> struct GetSigned<int> { using type = signed int; };
	template <> struct GetSigned<unsigned int> { using type = signed int; };
	template <> struct GetSigned<long> { using type = signed long; };
	template <> struct GetSigned<unsigned long> { using type = signed long; };
	template <> struct GetSigned<long long> { using type = signed long long; };
	template <> struct GetSigned<unsigned long long> { using type = signed long long; };

	template <bool Test, class Type0, class Type1> struct ConditionalOf { using type = Type0; };
	template <class Type0, class Type1> struct ConditionalOf<false, Type0, Type1> { using type = Type1; };

	template <bool Test, class Type = void> struct EnableIf {};
	template <class Type> struct EnableIf<true, Type> { using type = Type; };

	template <class Type> struct IsDefined
	{
		template <class U>
		static auto Test(U*) -> BoolConstant<sizeof(U) == sizeof(U)>;
		static auto Test(...) -> FalseConstant;
		using type = decltype(Test((Type*)0));
	};
}

template <class Type>
using Defined = typename TypeTraits::IsDefined<Type>::type;

template <bool Test, class Type0, class Type1>
using Conditional = typename TypeTraits::ConditionalOf<Test, Type0, Type1>::type;

namespace TypeTraits
{
	template<class Type, class... Rest>
	struct IsAnyOf : FalseConstant {};

	template<class Type, class First, class... Rest>
	struct IsAnyOf<Type, First, Rest...> : Conditional<IsSame<Type, First>, TrueConstant, IsAnyOf<Type, Rest...>> {};

	template<class Type>
	struct AppliedPointers { using type = Type; };

	template<class Type, U64 Count>
	struct ApplyPointers : Conditional<Count == 0, AppliedPointers<Type>, ApplyPointers<Type*, Count - 1>> {};

	template<class Type>
	struct IsDestroyable
	{
		template<class C> static constexpr TrueConstant Test(decltype(&C::Destroy));
		template<class> static constexpr FalseConstant Test(...);

		static constexpr bool value = decltype(Test<Type>(0))::value;
	};

	template<class From, class To>
	struct IsConvertableTo
	{
		template<class C> static constexpr TrueConstant Test(decltype(&C::operator To));
		template<class> static constexpr FalseConstant Test(...);

		static constexpr bool value = decltype(Test<From>(0))::value;
	};
}

template <bool Test, class Type = void>
using Enable = typename TypeTraits::EnableIf<Test, Type>::type;

template<class Type, class... Rest>
inline constexpr bool AnyOf = TypeTraits::IsAnyOf<Type, Rest...>::value;

template <class Type> using RemovedConst = typename TypeTraits::RemoveConst<Type>::type;
template <class Type> using RemovedVolatile = typename TypeTraits::RemoveVolatile<Type>::type;
template <class Type> using RemovedQuals = typename TypeTraits::RemoveQuals<Type>::type;
template <class Type> using RemovedReference = typename TypeTraits::RemoveReference<Type>::type;
template <class Type> using RemovedQualsReference = typename TypeTraits::RemoveQuals<RemovedReference<Type>>::type;
template <class Type> using RemovedPointer = typename TypeTraits::RemovePointer<Type>::type;
template <class Type> using AddedPointer = typename TypeTraits::AddPointer<Type>::type;
template <class Type> using RemovedPointers = typename TypeTraits::RemovePointerAll<Type>::type;
template <class Type> using RemovedArray = typename TypeTraits::RemoveArray<Type>::type;
template <class Type> using RemovedArrays = typename TypeTraits::RemoveArrayAll<Type>::type;
template <class Type> using AddLvalReference = typename TypeTraits::AddReference<RemovedReference<Type>>::lValue;
template <class Type> using AddRvalReference = typename TypeTraits::AddReference<RemovedReference<Type>>::rValue;
template <class Type> using BaseType = typename TypeTraits::RemoveQuals<RemovedPointers<RemovedArrays<RemovedReference<Type>>>>::type;
template <class Type> using UnsignedOf = typename TypeTraits::GetUnsigned<Type>::type;
template <class Type> using SignedOf = typename TypeTraits::GetSigned<Type>::type;
template <class Type> inline constexpr U64 PointerCount = TypeTraits::GetPointerCount<Type, 0>::count;
template <class Type, U64 Count> using ApplyPointers = typename TypeTraits::ApplyPointers<Type, Count>::type;

template <class Type> inline constexpr bool IsDestroyable = TypeTraits::IsDestroyable<Type>::value;
template <class Type> concept Destroyable = IsDestroyable<Type>;

template <class From, class To> inline constexpr bool ConvertibleTo = TypeTraits::IsConvertableTo<From, To>::value;

template <class Type> inline constexpr bool IsVoid = IsSame<RemovedQuals<Type>, void>;

template <class> inline constexpr bool IsPointer = false;
template <class Type> inline constexpr bool IsPointer<Type*> = true;
template <class Type> inline constexpr bool IsPointer<const Type*> = true;
template <class Type> inline constexpr bool IsPointer<volatile Type*> = true;
template <class Type> inline constexpr bool IsPointer<const volatile Type*> = true;
template <class Type> concept Pointer = IsPointer<Type>;
template <class Type> concept NonPointer = !IsPointer<Type>;

template <class Type> inline constexpr bool IsSinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;
template <class Type> concept SinglePointer = IsSinglePointer<Type>;

template <class> inline constexpr bool IsLReference = false;
template <class Type> inline constexpr bool IsLReference<Type&> = true;
template <class Type> inline constexpr bool IsLReference<const Type&> = true;
template <class Type> inline constexpr bool IsLReference<volatile Type&> = true;
template <class Type> inline constexpr bool IsLReference<const volatile Type&> = true;
template <class Type> concept LvalReference = IsLReference<Type>;

template <class> inline constexpr bool IsRReference = false;
template <class Type> inline constexpr bool IsRReference<Type&&> = true;
template <class Type> inline constexpr bool IsRReference<const Type&&> = true;
template <class Type> inline constexpr bool IsRReference<volatile Type&&> = true;
template <class Type> inline constexpr bool IsRReference<const volatile Type&&> = true;
template <class Type> concept RvalReference = IsRReference<Type>;

template <class Type> inline constexpr bool IsReference = IsLReference<Type> || IsRReference<Type>;
template <class Type> concept Reference = IsReference<Type>;

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
template <class Type> concept SingleArray = IsSingleArray<Type>;

template <class Type> inline constexpr bool IsCharacter = AnyOf<RemovedQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;
template <class Type> concept Character = IsCharacter<Type>;

template <class Type> inline constexpr bool IsBoolean = AnyOf<RemovedQuals<Type>, bool>;
template <class Type> concept Boolean = IsBoolean<Type>;

template <class Type> inline constexpr bool IsInteger = AnyOf<RemovedQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int,
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;
template <class Type> concept Integer = IsInteger<Type>;

template <class Type> inline constexpr bool IsSigned = AnyOf<RemovedQuals<Type>, signed char, short, signed short, int, signed int, long, signed long, long long, signed long long>;
template <class Type> concept Signed = IsSigned<Type>;

template <class Type> inline constexpr bool IsUnsigned = AnyOf<RemovedQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
template <class Type> concept Unsigned = IsUnsigned<Type>;

template <class Type> inline constexpr bool IsFloatingPoint = AnyOf<RemovedQuals<Type>, float, double, long double>;
template <class Type> concept FloatingPoint = IsFloatingPoint<Type>;

template <class Type> inline constexpr bool IsNumber = IsInteger<Type> || IsFloatingPoint<Type>;
template <class Type> concept Number = IsNumber<Type>;

template <class Type> inline constexpr bool IsStringLiteral = AnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);
template <class Type> concept StringLiteral = IsStringLiteral<Type>;

template <class Type> inline constexpr bool IsFunctionPtr = !IsConst<const Type> && !IsReference<Type>;
template <class Type> concept FunctionPtr = IsFunctionPtr<Type>;

/// <summary>
/// Forwards arg as movable
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template<class Type> constexpr RemovedReference<Type>&& Move(Type&& arg) noexcept { return static_cast<RemovedReference<Type>&&>(arg); }

/// <summary>
/// Forwards an lValue as an rValue
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template <class Type> constexpr Type&& Forward(RemovedReference<Type>& arg) noexcept { return static_cast<Type&&>(arg); }

/// <summary>
/// Forwards an lValue as an rValue
/// </summary>
/// <param name="arg:">The value to forward</param>
/// <returns>The forwarded value</returns>
template <class Type> constexpr Type&& Forward(RemovedReference<Type>&& arg) noexcept { static_assert(!IsLReference<Type>, "Bad Forward Call"); return static_cast<Type&&>(arg); }

template<class Type> inline constexpr void Swap(Type& a, Type& b) noexcept
{
	Type tmp = Move(a);
	a = Move(b);
	b = Move(tmp);
}

template<typename T> typename AddRvalReference<T> DeclValue() noexcept { static_assert(False<T>, "GetReference not allowed in an evaluated context"); }

namespace TypeTraits
{
	template <bool> struct Select { template <class Type1, class> using Apply = Type1; };
	template <> struct Select<false> { template <class, class Type2> using Apply = Type2; };

	template <class Type> struct GetDecay
	{
		using Type1 = RemovedReference<Type>;
		using Type2 = typename Select<IsFunctionPtr<Type1>>::template Apply<AddedPointer<Type1>, RemovedQuals<Type1>>;
		using type = typename Select<IsArray<Type1>>::template Apply<AddedPointer<RemovedArray<Type1>>, Type2>::type;
	};
}

template <class Type> using Decay = typename TypeTraits::GetDecay<Type>::type;

template <class Type1, class Type2> using ConditionalType = decltype(false ? DeclValue<Type1>() : DeclValue<Type2>());

template <class Type1, class Type2, class = void> struct ConstLValueCondition {};

template <class Type1, class Type2>
struct ConstLValueCondition<Type1, Type2, Void<ConditionalType<const Type1&, const Type2&>>> {
	using type = RemovedQualsReference<ConditionalType<const Type1&, const Type2&>>;
};

template <class Type1, class Type2, class = void>
struct DecayedCondition : ConstLValueCondition<Type1, Type2> {};

template <class Type1, class Type2>
struct DecayedCondition<Type1, Type2, Void<ConditionalType<Type1, Type2>>> {
	using type = Decay<ConditionalType<Type1, Type2>>;
};

namespace TypeTraits
{
	template <class... Types> struct GetCommonType;
}

template <class... Types> using CommonType = typename TypeTraits::GetCommonType<Types...>::type;

namespace TypeTraits
{
	template <> struct GetCommonType<> {};

	template <class Type1>
	struct GetCommonType<Type1> : GetCommonType<Type1, Type1> {};

	template <class Type1, class Type2, class Decayed1 = Decay<Type1>, class Decayed2 = Decay<Type2>>
	struct GetCommonType2 : GetCommonType<Decayed1, Decayed2> {};

	template <class Type1, class Type2>
	struct GetCommonType2<Type1, Type2, Type1, Type2> : DecayedCondition<Type1, Type2> {};

	template <class Type1, class Type2>
	struct GetCommonType<Type1, Type2> : GetCommonType2<Type1, Type2> {};

	template <class Void, class Type1, class Type2, class... Rest>
	struct GetCommonType3 {};

	template <class Type1, class Type2, class... Rest>
	struct GetCommonType3<Void<CommonType<Type1, Type2>>, Type1, Type2, Rest...> : GetCommonType<CommonType<Type1, Type2>, Rest...> {};

	template <class Type1, class Type2, class... Rest>
	struct GetCommonType<Type1, Type2, Rest...> : GetCommonType3<void, Type1, Type2, Rest...> {};
}

template <Integer I, I... Indices>
struct IntegerSequence
{
	using Type = I;

	NH_NODISCARD static constexpr U64 Size() noexcept
	{
		return sizeof...(Indices);
	}
};

template <U64... Indices> using IndexSequence = IntegerSequence<U64, Indices...>;
template <Integer I, I Size> using CreateIntegerSequence = __make_integer_seq<IntegerSequence, I, Size>;
template <U64 Size> using CreateIndexSequence = CreateIntegerSequence<U64, Size>;

template <Character C, C... Chars>
struct CharPack
{
	static constexpr C pack[sizeof...(Chars)] = { Chars... };
};

template<class Dest, class Src>
constexpr Dest CharCast(Src x)
{
	return static_cast<Dest>(x);
}

//template <class Func, class Type> inline constexpr bool Returns = ReturnType<Func> == Type;
//template <class Func> concept VoidFunction = Returns<Func, void>;
//template <class Func, class Type> concept TypeFunction = Returns<Func, Type>;

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

	static constexpr U64 GetNumericalBits()
	{
		if constexpr (IsSame<Base, char>) { return 8; }
		if constexpr (IsSame<Base, unsigned char>) { return 8; }
		if constexpr (IsSame<Base, signed char>) { return 7; }
		if constexpr (IsSame<Base, char8_t>) { return 8; }

#ifdef PLATFORM_WINDOWS
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

public:
	static constexpr Base MaxValue = GetMaxValue();
	static constexpr Base MinValue = GetMinValue();
	static constexpr U64 NumericalBits = GetNumericalBits();
	static constexpr Base Infinity = GetInfinity();
	static constexpr Base NaN = GetNaN();
	static constexpr Base Epsilon = GetEpsilon();
};

namespace TypeTraits
{
#if defined(_M_ARM) || defined(_M_ARM64)
#ifdef __clang__
	constexpr U64 ClangLeftZeroBitsArmArm64(const unsigned short val)
	{
		return __builtin_clzs(val);
	}

	constexpr U64 ClangLeftZeroBitsArmArm64(const unsigned int val)
	{
		return __builtin_clz(val);
	}

	constexpr U64 ClangLeftZeroBitsArmArm64(const unsigned long val)
	{
		return __builtin_clzl(val);
	}

	constexpr U64 ClangLeftZeroBitsArmArm64(const unsigned long long val)
	{
		return __builtin_clzll(val);
	}
#endif

	template <Unsigned T>
	U64 LeftZeroBitsArmArm64(const T val) noexcept
	{
		constexpr U64 bits = Traits<T>::NumericalBits;
		if (val == 0) { return bits; }

#ifdef __clang__
		if constexpr (IsSame<RemovedQuals<T>, char>, unsigned char > )
		{
			return ClangLeftZeroBitsArmArm64(static_cast<unsigned short>(val))
				- (Traits<unsigned short>::NumericalBits - bits);
		}
		else { return ClangLeftZeroBitsArmArm64(val); }
#else
		if constexpr (bits <= 32) { return static_cast<int>(_CountLeadingZeros(val)) - (Traits<unsigned long>::NumericalBits - bits); }
		else { return static_cast<int>(_CountLeadingZeros64(val)); }
#endif
	}
#endif

	template <class T>
	constexpr U64 LeftZeroBitsFallback(T val) noexcept
	{
		T shiftVal = 0;

		U64 bits = Traits<T>::NumericalBits;
		U64 halfBits = Traits<T>::NumericalBits / 2;
		do
		{
			shiftVal = static_cast<T>(val >> halfBits);
			if (shiftVal != 0)
			{
				bits -= halfBits;
				val = shiftVal;
			}
			halfBits >>= 1;
		} while (halfBits != 0);
		return static_cast<U64>(bits - val);
	}

	extern "C" { extern int __isa_available; }

	template <Unsigned T>
	U64 LeftZeroBitsLzcnt(const T val) noexcept
	{
		constexpr U64 bits = Traits<T>::NumericalBits;

		if constexpr (bits <= 16) { return static_cast<U64>(__lzcnt16(val) - (16 - bits)); }
		else if constexpr (bits == 32) { return static_cast<U64>(__lzcnt(val)); }
		else
		{
#ifdef _M_IX86
			const unsigned int high = val >> 32;
			if (high != 0) { return LeftZeroBitsLzcnt(high); }

			const unsigned int low = static_cast<unsigned int>(val);
			return 32 + LeftZeroBitsLzcnt(low);
#else
			return __lzcnt64(val);
#endif
		}
	}

	template <Unsigned T>
	U64 LeftZeroBitsBsr(const T val) noexcept
	{
		constexpr U64 bits = Traits<T>::NumericalBits;

		unsigned long result;
		if constexpr (bits <= 32)
		{
			if (!_BitScanReverse(&result, val)) { return bits; }
		}
		else
		{
#ifdef _M_IX86
			const unsigned int high = val >> 32;
			if (_BitScanReverse(&result, high)) { return static_cast<U64>(31 - result); }

			const auto low = static_cast<unsigned int>(val);
			if (!_BitScanReverse(&result, low)) { return bits; }
#else
			if (!_BitScanReverse64(&result, val)) { return bits; }
#endif
		}
		return static_cast<U64>(bits - 1 - result);
	}

	template <Unsigned T>
	U64 LeftZeroBitsx64x86(const T val) noexcept
	{
#ifdef __AVX2__
		return LeftZeroBitsLzcnt(val);
#else
		const bool hasLzcnt = __isa_available >= ISA_AVAILABLE_AVX2;

		if (hasLzcnt) { return LeftZeroBitsLzcnt(val); }
		else { return LeftZeroBitsBsr(val); }
#endif
	}
}

template <Unsigned T>
constexpr U64 LeftZeroBits(const T val) noexcept
{
	if (!ConstantEvaluation())
	{
#if defined(_M_IX86) || (defined(_M_X64) && !defined(_M_ARM64EC))
		return TypeTraits::LeftZeroBitsx64x86(val);
#elif defined(_M_ARM) || defined(_M_ARM64)
		return TypeTraits::LeftZeroBitsArmArm64(val);
#endif
	}

	return TypeTraits::LeftZeroBitsFallback(val);
}

template<Unsigned T>
NH_NODISCARD constexpr T BitCeiling(const T val) noexcept
{
	if (val <= 1u) { return T{ 1 }; }

	return static_cast<T>(T{ 1 } << (Traits<T>::NumericalBits - LeftZeroBits(static_cast<T>(val - 1))));
}

template<Unsigned T>
NH_NODISCARD constexpr T BitFloor(const T val) noexcept
{
	if (val == 0u) { return val; }

	return static_cast<T>(T{ 1 } << (Traits<T>::NumericalBits - 1 - LeftZeroBits(val)));
}

template<Unsigned T>
NH_NODISCARD constexpr T DegreeOfTwo(const T val) noexcept
{
	if (val <= 1u) { return T{ 0 }; }

	return static_cast<T>(Traits<T>::NumericalBits - 1 - LeftZeroBits(val));
}