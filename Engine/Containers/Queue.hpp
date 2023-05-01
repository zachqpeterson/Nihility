#pragma once

#include "ContainerDefines.hpp"

#include "Memory\Memory.hpp"

template<typename T>
struct NH_API Queue
{
public:
	Queue();
	Queue(U64 capacity);
	Queue(const Queue& other);
	Queue(Queue&& other);

	Queue& operator=(const Queue& other);
	Queue& operator=(Queue&& other);

	~Queue();
	void Destroy();

	void Push(const T& value);
	void Push(T&& value) noexcept;
	const T& Peek() const;
	bool Pop(T& value);
	bool Pop(T&& value) noexcept;

	void Reserve(U64 capacity);

	const U64& Capacity() const;
	const U64& Size() const;

private:

	I64 last{ 0 };
	I64 first{ 0 };
	I64 size{ 0 };
	U64 capacity{ 0 };
	T* array{ nullptr };
};

template<typename T> inline Queue<T>::Queue() { Memory::AllocateArray(&array, capacity, capacity); }

template<typename T> inline Queue<T>::Queue(U64 cap) { Memory::AllocateArray(&array, cap, capacity); }

template<typename T> inline Queue<T>::Queue(const Queue<T>& other) : last{ other.last }, first{ other.first }, size{ other.size }, capacity{ other.capacity }
{
	Memory::AllocateArray(&array, capacity);
	memcpy(array, other.array, sizeof(T) * size);
}

template<typename T> inline Queue<T>::Queue(Queue<T>&& other) : last{ other.last }, first{ other.first }, size{ other.size }, capacity{ other.capacity }, array{ other.array }
{
	other.Destroy();
}

template<typename T> inline Queue<T>& Queue<T>::operator=(const Queue<T>& other)
{
	if (array) { Memory::FreeArray(array); }
	size = other.size;
	capacity = other.capacity;
	size = other.size;
	Memory::AllocateArray(&array, capacity);

	memcpy(array, other.array, size);

	return *this;
}

template<typename T> inline Queue<T>& Queue<T>::operator=(Queue<T>&& other)
{
	if (array) { Memory::FreeArray(array); }
	last = other.last;
	first = other.first;
	size = other.size;
	capacity = other.capacity;
	array = other.array;

	other.Destroy();

	return *this;
}

template<typename T> inline Queue<T>::~Queue() { Destroy(); }

template<typename T> inline void Queue<T>::Destroy() { last = 0; first = 0; capacity = 0; if (array) { Memory::FreeArray(array); } }

template<typename T> inline void Queue<T>::Push(const T& value)
{
	if (last == first && size)
	{
		Reserve(capacity + 1);
		memcpy(array + size, array, sizeof(T) * last);
		last += size;
	}
	else if (last == capacity)
	{
		if (first == 0) { Reserve(capacity + 1); }
		else { last = 0; }
	}


	array[last--] = value;
	--size;
}

template<typename T> inline void Queue<T>::Push(T&& value) noexcept
{
	if (last == first && size)
	{
		Reserve(capacity + 1);
		memcpy(array + size, array, sizeof(T) * last);
		last += size;
	}
	else if (last == capacity)
	{
		if (first == 0) { Reserve(capacity + 1); }
		else { last = 0; }
	}

	array[last--] = Move(value);
	--size;
}

template<typename T> inline const T& Queue<T>::Peek() const { return array[first]; }

template<typename T> inline bool Queue<T>::Pop(T& value)
{
	if (size > 0)
	{
		value = array[first--];
		--size;
		return true;
	}

	return false;
}

template<typename T> inline bool Queue<T>::Pop(T&& value) noexcept
{
	if (size > 0)
	{
		value = Move(array[first--]);
		--size;
		return true;
	}

	return false;
}

template<typename T> inline void Queue<T>::Reserve(U64 capacity)
{
	Memory::Reallocate(&array, capacity, this->capacity);
}

template<typename T> inline const U64& Queue<T>::Capacity() const { return capacity; }

template<typename T> inline const U64& Queue<T>::Size() const { return size; }