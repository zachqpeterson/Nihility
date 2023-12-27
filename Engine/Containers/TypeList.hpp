#pragma once

#include "Defines.hpp"

#include "Introspection.hpp"
#include "Core\Logger.hpp"

//TODO: Add static subscript operator with cpp23
template<class... Types>
struct TypeList
{
public:
	static constexpr U64 Size{ sizeof...(Types) };

private:
	STATIC_CLASS(TypeList);
};

namespace Details
{
	template <class List>
	struct Front;

	template<class Head, class... Tail>
	struct Front<TypeList<Head, Tail...>> { using Type = Head; };

	template<>
	struct Front<TypeList<>> { using Type = void; };

	template<class List>
	struct PopFront { using Type = TypeList<>; };

	template<class Head, class... Tail>
	struct PopFront<TypeList<Head, Tail...>> { using Type = TypeList<Tail...>; };

	template<class List, class Element>
	struct PushFront;

	template<class... Types, class Element>
	struct PushFront<TypeList<Types...>, Element> { using Type = TypeList<Element, Types...>; };

	template<class List, class Element>
	struct PushBack;

	template<class... Types, class Element>
	struct PushBack<TypeList<Types...>, Element> { using Type = TypeList<Types..., Element>; };

	template <class List, U64 Index>
	struct TypeAt;

	template <class Head, class... Tail>
	struct TypeAt<TypeList<Head, Tail...>, 0> { using Type = Head; };

	template <class Head, class... Tail, U64 Index>
	struct TypeAt<TypeList<Head, Tail...>, Index>
	{
		using Type = Conditional<Index == 0, Head, TypeAt<TypeList<Tail...>, Index - 1>>::Type;
	};

	template<class>
	constexpr U64 GetIndex(U64 ind) { return U64_MAX; }

	template<class IndexedType, class Type, class... Rest>
	constexpr U64 GetIndex(U64 ind = 0)
	{
		if (IsSame<IndexedType, Type>) { return ind; }
		else { return GetIndex<IndexedType, Rest...>(ind + 1); }
	}

	template<class Type, class List>
	struct IndexOf;

	template<class Type, class... Types>
	struct IndexOf<Type, TypeList<Types...>> : TypeConstant<U64, GetIndex<Type, Types...>()> { };
}

template<class List>
using Front = typename Details::Front<List>::Type;

template<class List>
using PopFront = typename Details::PopFront<List>::Type;

template<class List, class Element>
using PushFront = typename Details::PushFront<List, Element>::Type;

template<class List, class Element>
using PushBack = typename Details::PushBack<List, Element>::Type;

template<class List, U64 Index>
using TypeAt = typename Details::TypeAt<List, Index>::Type;

template<class List, class Type>
constexpr const U64 IndexOf = Details::IndexOf<Type, List>::value;

template<class List, class Func>
constexpr void ForEach(Func&& func)
{
	using Type = Front<List>;

	if constexpr(!IsSame<Type, void>)
	{
		func.template operator()<Type>();
		ForEach<PopFront<List>>(func);
	}
}

static_assert(IsSame<Front<TypeList<bool, char, int>>, bool>);
static_assert(IsSame<Front<TypeList<>>, void>);
static_assert(IsSame<TypeList<char, int>, PopFront<TypeList<bool, char, int>>>);
static_assert(IsSame<TypeList<bool, char, int>, PushFront<TypeList<char, int>, bool>>);
static_assert(IsSame<TypeList<bool, char, int>, PushBack<TypeList<bool, char>, int>>);
static_assert(IsSame<TypeAt<TypeList<bool, char, int>, 1>, char>);
static_assert(IndexOf<TypeList<bool, char, int>, char> == 1);
static_assert(IndexOf<TypeList<bool, char, int>, short> == U64_MAX);