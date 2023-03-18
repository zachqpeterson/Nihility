#pragma once

#include "ContainerDefines.hpp"

#include "Memory\Memory.hpp"

template<typename T>
struct Queue
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

	T* array;
	volatile I64 last;
	volatile I64 first;
	volatile I64 size;
	volatile U64 capacity;
};

template<typename T> inline Queue<T>::Queue() : array{ (T*)Memory::Allocate1kb() }, last{ 0 }, first{ 0 }, size{ 0 }, capacity{ 1024 / sizeof(T) } {}

template<typename T> inline Queue<T>::Queue(U64 cap) : array{ (T*)Memory::Allocate(sizeof(T) * cap, capacity) }, last{ 0 }, first{ 0 }, size{ 0 }, capacity{ capacity / sizeof(T) } {}

template<typename T> inline Queue<T>::Queue(const Queue<T>& other) : array{ (T*)Memory::Allocate(sizeof(T) * capacity) }, 
last{ other.last }, first{ other.first }, size{ other.size }, capacity{ other.capacity }
{
	memcpy(array, other.array, sizeof(T)* size);
}

template<typename T> inline Queue<T>::Queue(Queue<T>&& other) : array{ other.array }, last{ other.last }, first{ other.first }, size{ other.size }, capacity{ other.capacity }
{
	other.Destroy();
}

template<typename T> inline Queue<T>& Queue<T>::operator=(const Queue<T>& other)
{
	if (array) { Memory::Free(array); }
	size = other.size;
	capacity = other.capacity;
	size = other.size;
	array = (T*)Memory::Allocate(sizeof(T) * capacity);

	memcpy(array, other.array, sizeof(T) * size);

	return *this;
}

template<typename T> inline Queue<T>& Queue<T>::operator=(Queue<T>&& other)
{
	if (array) { Memory::Free(array); }
	last = other.last;
	first = other.first;
	size = other.size;
	capacity = other.capacity;
	array = other.array;

	other.Destroy();

	return *this;
}

template<typename T> inline Queue<T>::~Queue() { Destroy(); }

template<typename T> inline void Queue<T>::Destroy() { last = 0; first = 0; capacity = 0; if (array) { Memory::Free(array); } array = nullptr; }

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
	U64 cap;
	T* temp = (T*)Memory::Allocate(sizeof(T) * capacity, cap);
	memcpy(temp, array, sizeof(T) * this->capacity);
	Memory::Free(array);
	array = temp;
	this->capacity = cap / sizeof(T);
}

template<typename T> inline const U64& Queue<T>::Capacity() const { return capacity; }

template<typename T> inline const U64& Queue<T>::Size() const { return size; }