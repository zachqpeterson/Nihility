#pragma once

#include "Defines.hpp"

#include <initializer_list>

template<class Type, U64 Capacity>
struct Array
{
public:
	Array(){}
	Array(std::initializer_list<Type> list);

	const Type& operator[](U64 i) const { return array[i]; }
	Type& operator[](U64 i) { return array[i]; }

	Type* begin() { return array; }
	Type* end() { return array + Capacity; }

private:
	Type array[Capacity];
};

template<class Type, U64 Capacity>
Array<Type, Capacity>::Array(std::initializer_list<Type> list)
{
	U64 i = 0;
	for (const Type& t : list) { array[i++] = t; }
}