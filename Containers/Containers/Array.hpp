#pragma once

#include "Defines.hpp"

template<typename T, U64 size>
struct NH_API Array
{
public:
	struct NH_API Iterator
	{
		Iterator(T* ptr) : ptr{ ptr } {}

		T& operator* () const { return *ptr; }
		T* operator-> () { return ptr; }

		Iterator& operator++ () { ++ptr; return *this; }
		Iterator operator++ (int)
		{
			Iterator temp = *this;
			++ptr;
			return temp;
		}

		Iterator& operator-- () { --ptr; return *this; }
		Iterator operator-- (int)
		{
			Iterator temp = *this;
			--ptr;
			return temp;
		}

		Iterator operator+(int i)
		{
			Iterator temp = *this;
			temp += i;
			return temp;
		}

		Iterator operator-(int i)
		{
			Iterator temp = *this;
			temp -= i;
			return temp;
		}

		Iterator& operator+=(int i)
		{
			ptr += i;
			return *this;
		}

		Iterator& operator-=(int i)
		{
			ptr -= i;
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

		operator bool() { return ptr; }

	private:
		T* ptr;
	};

public:
	Array() {}
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

	Array(const Array&) = delete;
	Array(Array&&) = delete;

	T* Data() { return array; }
	const T* Data() const { return array; }
	T& Front() { return *array; }
	const T& Front() const { return *array; }
	T& Back() { return array[size - 1]; }
	const T& Back() const { return array[size - 1]; }
	U64 Size() const { return size; }

	T& operator[](U64 i) { return array[i]; }
	const T& operator[](U64 i) const { return array[i]; }

	Iterator begin() { return Iterator{ array }; }
	Iterator end() { return Iterator{ &array[size] }; }
	Iterator begin() const { return Iterator{ array }; }
	Iterator end() const { return Iterator{ &array[size] }; }

private:
	T array[size];
};