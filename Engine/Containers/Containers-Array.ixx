module;

#include "Defines.hpp"

export module Containers:Array;

export template<class Type, U64 Capacity>
struct Array
{
public:
	Array(){}

	const Type& operator[](U64 i) const { return array[i]; }
	Type& operator[](U64 i) { return array[i]; }

private:
	Type array[Capacity];
};