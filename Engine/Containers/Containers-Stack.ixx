module;

#include "Defines.hpp"

export module Containers:Stack;

import Memory;

export template <class Type>
struct Stack
{
	Stack();
	Stack(U32 capacity);
	Stack(const Stack& other);
	Stack(Stack&& other) noexcept;

	Stack& operator=(const Stack& other);
	Stack& operator=(Stack&& other) noexcept;

	~Stack();
	void Destroy();
	void Clear();

	void Push(const Type& value);
	void Push(Type&& value) noexcept;
	bool Pop(Type& value);

	void Reserve(U32 capacity);

	U32 Capacity() const;
	U32 Size() const;
	bool Empty() const;
	bool Full() const;

private:
	U32 size = 0;
	U32 capacity = 0;
	Type* array = nullptr;
};

template <class Type>
inline Stack<Type>::Stack() { Memory::AllocateArray(&array, capacity, capacity); }

template <class Type>
inline Stack<Type>::Stack(U32 cap) : capacity(cap) { Memory::AllocateArray(&array, cap, capacity); }

template <class Type>
inline Stack<Type>::Stack(const Stack& other) : size(other.size), capacity(other.size)
{
	Memory::AllocateArray(&array, capacity, capacity);
	Copy(array, other.array, size);
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
	if (capacity < other.size) { Memory::Reallocate(&array, size, capacity); }

	Copy(array, other.array, size);

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
	size = 0;
	capacity = 0;
	Memory::Free(&array);
}

template <class Type>
inline void Stack<Type>::Clear() { size = 0; }

template <class Type>
inline void Stack<Type>::Push(const Type& value)
{
	if (Full()) { Reserve(capacity + 1); }

	array[size++] = value;
}

template <class Type>
inline void Stack<Type>::Push(Type&& value) noexcept
{
	if (Full()) { Reserve(capacity + 1); }

	array[size++] = Move(value);
}

template <class Type>
inline bool Stack<Type>::Pop(Type& value)
{
	if (Empty()) { return false; }

	value = Move(array[--size]);

	return true;
}

template <class Type>
inline void Stack<Type>::Reserve(U32 cap) { Memory::Reallocate(&array, cap, capacity); }

template <class Type>
inline U32 Stack<Type>::Capacity() const { return capacity; }

template <class Type>
inline U32 Stack<Type>::Size() const { return size; }

template <class Type>
inline bool Stack<Type>::Empty() const { return size == 0; }

template <class Type>
inline bool Stack<Type>::Full() const { return size == capacity; }