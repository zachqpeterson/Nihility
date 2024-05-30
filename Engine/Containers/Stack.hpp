#pragma once

#include "ContainerDefines.hpp"

template <class T>
struct Stack
{
	Stack();
	Stack(U32 capacity);
	Stack(U32 size, const T& value);
	Stack(U32 size, T* array);
	Stack(const Stack& other);
	Stack(Stack&& other) noexcept;

	Stack& operator=(const Stack& other);
	Stack& operator=(Stack&& other) noexcept;

	~Stack();
	void Destroy();
	void Clear();

	void Push(const T& value);
	void Push(T&& value) noexcept;

	const T& Peek() const;
	T& Peek();

	T Pop();

	void Resize(U32 size, const T& value);
	void Reserve(U32 capacity);

	bool Empty() const;
	U32 Size() const;
	U32 Capacity() const;

private:
	U32 size{ 0 };
	U32 capacity{ 0 };
	T* array{ nullptr };
};

template <class T>
inline Stack<T>::Stack() { Memory::AllocateArray(&array, capacity, capacity); }

template <class T>
inline Stack<T>::Stack(U32 cap) : capacity{ cap } { Memory::AllocateArray(&array, cap, capacity); }

template <class T>
inline Stack<T>::Stack(U32 size, const T& value) : size{ size }, capacity{ size }
{
	Memory::AllocateArray(&array, capacity, capacity);
	for (T* t = array, *end = array + size; t != end; ++t) { *t = value; }
}

template <class T>
inline Stack<T>::Stack(U32 size, T* arr) : size{ size }, capacity{ size }
{
	Memory::AllocateArray(&array, capacity, capacity);
	Memory::Copy(array, arr, sizeof(T) * size);
}

template <class T>
inline Stack<T>::Stack(const Stack& other) : size{ other.size }, capacity{ other.size }
{
	Memory::AllocateArray(&array, capacity, capacity);
	Memory::Copy(array, other.array, sizeof(T) * size);
}

template <class T>
inline Stack<T>::Stack(Stack&& other) noexcept : size{ other.size }, capacity{ other.capacity }, array{ other.array }
{
	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;
}

template <class T>
inline Stack<T>& Stack<T>::operator=(const Stack& other)
{
	size = other.size;
	if (capacity < other.size) { Memory::Reallocate(&array, size, capacity); }

	Memory::Copy(array, other.array, size * sizeof(T));

	return *this;
}

template <class T>
inline Stack<T>& Stack<T>::operator=(Stack&& other) noexcept
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

template <class T>
inline Stack<T>::~Stack()
{
	Destroy();
}

template <class T>
inline void Stack<T>::Destroy()
{
	size = 0; capacity = 0; Memory::Free(&array);
}

template <class T>
inline void Stack<T>::Clear()
{
	size = 0;
}

template <class T>
inline void Stack<T>::Push(const T& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[size++] = value;
}

template <class T>
inline void Stack<T>::Push(T&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[size++] = Move(value);
}

template <class T>
inline const T& Stack<T>::Peek() const
{
	if (size) { return array[size - 1]; }

	return array[size];
}

template <class T>
inline T& Stack<T>::Peek()
{
	if (size) { return array[size - 1]; }

	return array[size];
}

template <class T>
inline T Stack<T>::Pop()
{
	if (size) { return Move(array[--size]); }

	return {};
}

template <class T>
inline void Stack<T>::Resize(U32 size, const T& value)
{
	if (size > capacity) { Reserve(size); }
	this->size = size;

	for (U64 i = 0; i < size; ++i) { array[i] = value; }
}

template <class T>
inline void Stack<T>::Reserve(U32 cap)
{
	Memory::Reallocate(&array, cap, capacity);
}

template <class T>
inline bool Stack<T>::Empty() const
{
	return size == 0;
}

template <class T>
inline U32 Stack<T>::Size() const
{
	return size;
}

template <class T>
inline U32 Stack<T>::Capacity() const
{
	return capacity;
}