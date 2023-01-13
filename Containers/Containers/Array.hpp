#pragma once

#include "Defines.hpp"
#include "Memory/Memory.hpp"

template<typename T, U64 size>
struct Array
{
public:
	Array() = default;
	Array(const T& value)
	{
		for (U64 i = 0; i < size; ++i)
		{
			array[i] = value;
		}
	}
	Array(T&& value)
	{
		for (U64 i = 0; i < size; ++i)
		{
			array[i] = value;
		}
	}
	~Array() {}
	void Destroy() {}

	void* operator new(U64 sz) { return Memory::Allocate(sizeof(Array), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Array), MEMORY_TAG_DATA_STRUCT); }

	Array(const Array& other) = delete;
	Array(Array&& other)
	{
		array = other.array;
		other.array = nullptr;
	}

	T* Data() { return array; }
	const T* Data() const { return array; }
	T& Front() { return *array; }
	const T& Front() const { return *array; }
	T& Back() { return array[size - 1]; }
	const T& Back() const { return array[size - 1]; }
	U64 Size() const { return size; }

	T& operator[](U64 i) { return array[i]; }
	const T& operator[](U64 i) const { return array[i]; }

	T* begin() { return array; }
	T* end() { return array + size; }
	const T* begin() const { return array; }
	const T* end() const { return array + size; }

private:
	T array[size];
};