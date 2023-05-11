#pragma once

template <class Derived, class Base> inline constexpr bool InheritsFrom = __is_base_of(Base, Derived) && __is_convertible_to(const volatile Derived*, const volatile Base*);

template <class From, class To> inline constexpr bool ConvertibleTo = __is_convertible_to(From, To);

template <class Type> inline constexpr bool IsClass = __is_class(Type);
template <class Type> concept Class = IsClass<Type>;

template <class Type> inline constexpr bool IsUnion = __is_union(Type);
template <class Type> concept Union = IsUnion<Type>;

template <class Type> inline constexpr bool IsNothrowMoveConstructible = __is_nothrow_constructible(Type, Type);
template <class Type> concept NothrowMoveConstructible = IsNothrowMoveConstructible<Type>;

template <class Type> inline constexpr unsigned long long Alignment = alignof(Type);

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

template <class> inline constexpr bool True = true;
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

	template <class Type> struct IsMemberFunctionPtr { using type = FalseConstant; };

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
	template<typename Type, typename... Rest>
	struct IsAnyOf : FalseConstant {};

	template<typename Type, typename First, typename... Rest>
	struct IsAnyOf<Type, First, Rest...> : Conditional<IsSame<Type, First>, TrueConstant, IsAnyOf<Type, Rest...>> {};
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
template <class Type> using RemovedPointers = typename TypeTraits::RemovePointerAll<Type>::type;
template <class Type> using RemovedArray = typename TypeTraits::RemoveArray<Type>::type;
template <class Type> using RemovedArrays = typename TypeTraits::RemoveArrayAll<Type>::type;
template <class Type> using AddLvalReference = typename TypeTraits::AddReference<RemovedReference<Type>>::lValue;
template <class Type> using AddRvalReference = typename TypeTraits::AddReference<RemovedReference<Type>>::rValue;
template <class Type> using BaseType = typename TypeTraits::RemoveQuals<RemovedPointers<RemovedArrays<RemovedReference<Type>>>>::type;
template <class Type> using UnsignedOf = typename TypeTraits::GetUnsigned<Type>::type;
template <class Type> using SignedOf = typename TypeTraits::GetSigned<Type>::type;

template <class Type> inline constexpr bool IsVoid = IsSame<RemovedQuals<Type>, void>;

template <class> inline constexpr bool IsPointer = false;
template <class Type> inline constexpr bool IsPointer<Type*> = true;
template <class Type> inline constexpr bool IsPointer<const Type*> = true;
template <class Type> inline constexpr bool IsPointer<volatile Type*> = true;
template <class Type> inline constexpr bool IsPointer<const volatile Type*> = true;
template <class Type> concept Pointer = IsPointer<Type>;

template <class Type> inline constexpr bool IsSinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;
template <class Type> concept SinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;

template <class> inline constexpr bool IsLvalReference = false;
template <class Type> inline constexpr bool IsLvalReference<Type&> = true;
template <class Type> inline constexpr bool IsLvalReference<const Type&> = true;
template <class Type> inline constexpr bool IsLvalReference<volatile Type&> = true;
template <class Type> inline constexpr bool IsLvalReference<const volatile Type&> = true;
template <class Type> concept LvalReference = IsLvalReference<Type>;

template <class> inline constexpr bool IsRvalReference = false;
template <class Type> inline constexpr bool IsRvalReference<Type&&> = true;
template <class Type> inline constexpr bool IsRvalReference<const Type&&> = true;
template <class Type> inline constexpr bool IsRvalReference<volatile Type&&> = true;
template <class Type> inline constexpr bool IsRvalReference<const volatile Type&&> = true;
template <class Type> concept RvalReference = IsRvalReference<Type>;

template <class Type> inline constexpr bool IsReference = IsLReference<Type> || IsRReference<Type>;
template <class Type> concept Reference = IsLvalReference<Type> || IsRvalReference<Type>;

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

template <class Type> inline constexpr bool IsFunctionPtr = !IsConst<const Type> && !IsReference<Type>;
template <class Type> concept FunctionPtr = !IsConst<const Type> && !IsReference<Type>;

template <class Type> inline constexpr bool IsMemberFunctionPtr = TypeTraits::IsMemberFunctionPtr<RemovedQuals<Type>>::type::value;

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
template <class Type> constexpr Type&& Forward(RemovedReference<Type>&& arg) noexcept { static_assert(!IsLvalReference<Type>, "Bad Forward Call"); return static_cast<Type&&>(arg); }

template<class Type> inline constexpr void Swap(Type& a, Type& b)
{
	Type tmp = Move(a);
	a = Move(b);
	b = Move(tmp);
}

template<typename T> typename AddRvalReference<T> DeclValue() noexcept { static_assert(False<T>, "GetReference not allowed in an evaluated context"); }

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

public:
	static constexpr Base MaxValue = GetMaxValue();
	static constexpr Base MinValue = GetMinValue();
};