#pragma once

#include "Defines.hpp"

#include "Platform/Memory.hpp"

template <class Type>
struct Stack
{
	Stack();
	Stack(U64 capacity);
	Stack(const Stack& other);
	Stack(Stack&& other) noexcept;

	Stack& operator=(const Stack& other);
	Stack& operator=(Stack&& other) noexcept;

	~Stack();
	void Destroy();
	void Clear();

	void Push(const Type& value);
	void Push(Type&& value) noexcept;
	const Type& Peek() const;
	bool Pop(Type& value);

	void Reserve(U64 capacity);

	U64 Capacity() const;
	U64 Size() const;
	bool Empty() const;
	bool Full() const;

private:
	U64 size = 0;
	U64 capacity = 0;
	Type* array = nullptr;
};

template <class Type>
inline Stack<Type>::Stack() { }

template <class Type>
inline Stack<Type>::Stack(U64 cap) { capacity = Memory::Allocate(&array, cap); }

template <class Type>
inline Stack<Type>::Stack(const Stack& other) : size(other.size), capacity(other.size)
{
	capacity = Memory::Allocate(&array, capacity);
	CopyData(array, other.array, size);
}

template <class Type>
inline Stack<Type>::Stack(Stack&& other) noexcept : size(other.size), capacity(other.capacity), array(other.array)
{
	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;
}

template <class Type>
inline Stack<Type>& Stack<Type>::operator=(const Stack& other)
{
	size = other.size;
	if (capacity < other.size) { capacity = Memory::Reallocate(&array, size); }

	CopyData(array, other.array, size);

	return *this;
}

template <class Type>
inline Stack<Type>& Stack<Type>::operator=(Stack&& other) noexcept
{
	if (array) { Memory::Free(&array); }
	size = other.size;
	capacity = other.capacity;
	array = other.array;

	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;

	return *this;
}

template <class Type>
inline Stack<Type>::~Stack() { Destroy(); }

template <class Type>
inline void Stack<Type>::Destroy()
{
	if (array)
	{
		if constexpr (IsDestructible<Type>)
		{
			for (Type* it = array, *end = array + size; it != end; ++it) { it->~Type(); }
		}

		Memory::Free(&array);
	}

	size = 0;
	capacity = 0;
}

template <class Type>
inline void Stack<Type>::Clear() { size = 0; }

template <class Type>
inline void Stack<Type>::Push(const Type& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	Construct<Type>(array + size++, value);
}

template <class Type>
inline void Stack<Type>::Push(Type&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	Construct<Type>(array + size++, Move(value));
}

template <class Type>
inline const Type& Stack<Type>::Peek() const
{
	return array[size - 1];
}

template <class Type>
inline bool Stack<Type>::Pop(Type& value)
{
	if (size) { Construct<Type>(&value, Move(array[--size])); return true; }

	return false;
}

template <class Type>
inline void Stack<Type>::Reserve(U64 cap) { capacity = Memory::Reallocate(&array, cap); }

template <class Type>
inline U64 Stack<Type>::Capacity() const { return capacity; }

template <class Type>
inline U64 Stack<Type>::Size() const { return size; }

template <class Type>
inline bool Stack<Type>::Empty() const { return size == 0; }

template <class Type>
inline bool Stack<Type>::Full() const { return size == capacity; }