#pragma once

#include "ContainerDefines.hpp"

template<class Type, U32 index>
struct TupleLeaf
{
	[[no_unique_address]] Type value;
};

template<U32 index, class Type, class... Rest>
struct TupleBase : TupleLeaf<Type, index>, TupleBase<index + 1, Rest...>
{

};
