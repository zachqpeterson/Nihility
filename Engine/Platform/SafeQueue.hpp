#pragma once

#include "Defines.hpp"

template <typename Type>
struct SafeQueue
{
public:
	bool Push(const Type& value)
	{
		if (write + 1 == read) { return false; }

		array[write] = value;
		_WriteBarrier();
		++write;

		return true;
	}

	bool Pop(Type& value)
	{
		if (read == write) { return false; }

		value = array[read];
		_ReadWriteBarrier();
		++read;

		return true;
	}

private:
	Type array[256];
	U8 read{ 0 };
	U8 write{ 0 };
};