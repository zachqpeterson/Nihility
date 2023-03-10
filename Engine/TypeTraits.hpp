#pragma once

namespace Traits
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

	template <bool Value, class First, class... Rest>
	struct _Disjunction { using type = First; };

	template <class False, class Next, class... Rest>
	struct _Disjunction<false, False, Next, Rest...> { using type = typename _Disjunction<Next::value, Next, Rest...>::type; };

	template <class... Empty>
	struct disjunction { static constexpr bool value = false; };

	template <class First, class... Rest>
	struct disjunction<First, Rest...> : _Disjunction<First::value, First, Rest...>::type { };

	template <class... Traits>
	inline constexpr bool disjunction_v = disjunction<Traits...>::value;
}

template <class Type, class... Types>
inline constexpr bool IsAnyOf = Traits::disjunction_v<IsSame<Type, Types>...>;

template <class Type> using RemovedConst = typename Traits::RemoveConst<Type>::type;
template <class Type> using RemovedVolatile = typename Traits::RemoveVolatile<Type>::type;
template <class Type> using RemovedQuals = typename Traits::RemoveQuals<Type>::type;
template <class Type> using RemovedReference = typename Traits::RemoveReference<Type>::type;
template <class Type> using RemovedPointer = typename Traits::RemovePointer<Type>::type;
template <class Type> using RemovedPointers = typename Traits::RemovePointerAll<Type>::type;
template <class Type> using RemovedArray = typename Traits::RemoveArray<Type>::type;
template <class Type> using RemovedArrays = typename Traits::RemoveArrayAll<Type>::type;
template <class Type> using BaseType = typename Traits::RemoveQuals<RemovedPointers<RemovedArrays<RemovedReference<Type>>>>::type;

template <class> inline constexpr bool IsPointer = false;
template <class Type> inline constexpr bool IsPointer<Type*> = true;
template <class Type> inline constexpr bool IsPointer<const Type*> = true;
template <class Type> inline constexpr bool IsPointer<volatile Type*> = true;
template <class Type> inline constexpr bool IsPointer<const volatile Type*> = true;
template <class Type> concept Pointer = IsPointer<Type>;

template <class Type> inline constexpr bool IsSinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;
template <class Type> concept SinglePointer = IsPointer<Type> && !IsPointer<RemovedPointer<Type>>;

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

template <class, class> inline constexpr bool IsSame = false;
template <class Type> inline constexpr bool IsSame<Type, Type> = true;

template <class Type> inline constexpr bool IsCharacter = IsAnyOf<RemovedQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;
template <class Type> concept Character = IsAnyOf<RemovedQuals<Type>, char, wchar_t, char8_t, char16_t, char32_t>;

template <class Type> inline constexpr bool IsBoolean = IsAnyOf<RemovedQuals<Type>, bool>;
template <class Type> concept Boolean = IsAnyOf<RemovedQuals<Type>, bool>;

template <class Type> inline constexpr bool IsInteger = IsAnyOf<RemovedQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int,
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;
template <class Type> concept Integer = IsAnyOf<RemovedQuals<Type>, signed char, unsigned char, short, signed short, unsigned short, int, signed int, unsigned int, 
	long, signed long, unsigned long, long long, signed long long, unsigned long long>;

template <class Type> inline constexpr bool IsSigned = IsAnyOf<RemovedQuals<Type>, short, signed short, int, signed int, long, signed long, long long, signed long long>;
template <class Type> concept Signed = IsAnyOf<RemovedQuals<Type>, short, signed short, int, signed int, long, signed long, long long, signed long long>;

template <class Type> inline constexpr bool IsUnsigned = IsAnyOf<RemovedQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
template <class Type> concept Unsigned = IsAnyOf<RemovedQuals<Type>, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class Type> inline constexpr bool IsFloatingPoint = IsAnyOf<RemovedQuals<Type>, float, double, long double>;
template <class Type> concept FloatingPoint = IsAnyOf<RemovedQuals<Type>, float, double, long double>;

template <class Type> inline constexpr bool IsStringLiteral = IsAnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);
template <class Type> concept StringLiteral = IsAnyOf<BaseType<Type>, char, wchar_t, char8_t, char16_t, char32_t> && (IsSinglePointer<Type> || IsSingleArray<Type>);