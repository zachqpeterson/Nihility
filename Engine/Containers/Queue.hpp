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

	U64 Capacity() const;
	U64 Size() const;

private:

	U64 front{ 0 };
	U64 back{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	T* array{ nullptr };
};

template<typename T> inline Queue<T>::Queue() { Memory::AllocateArray(&array, capacity, capacity); }

template<typename T> inline Queue<T>::Queue(U64 cap) { Memory::AllocateArray(&array, cap, capacity); }

template<typename T> inline Queue<T>::Queue(const Queue<T>& other) : front{ other.front }, back{ other.back }, size{ other.size }, capacity{ other.capacity }
{
	Memory::AllocateArray(&array, capacity);
	Memory::Copy(array, other.array, sizeof(T) * size);
}

template<typename T> inline Queue<T>::Queue(Queue<T>&& other) : front{ other.front }, back{ other.back }, size{ other.size }, capacity{ other.capacity }, array{ other.array }
{
	other.Destroy();
}

template<typename T> inline Queue<T>& Queue<T>::operator=(const Queue<T>& other)
{
	if (array) { Memory::Free(array); }
	front = other.front;
	back = other.back;
	size = other.size;
	capacity = other.capacity;
	size = other.size;
	Memory::AllocateArray(&array, capacity);

	Memory::Copy(array, other.array, size);

	return *this;
}

template<typename T> inline Queue<T>& Queue<T>::operator=(Queue<T>&& other)
{
	if (array) { Memory::Free(array); }
	front = other.front;
	back = other.back;
	size = other.size;
	capacity = other.capacity;
	array = other.array;

	other.Destroy();

	return *this;
}

template<typename T> inline Queue<T>::~Queue() { Destroy(); }

template<typename T> inline void Queue<T>::Destroy() { front = 0; back = 0; size = 0; capacity = 0; if (array) { Memory::Free(&array); } }

template<typename T> inline void Queue<T>::Push(const T& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[front++] = value;
	++size;
}

template<typename T> inline void Queue<T>::Push(T&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[front++] = Move(value);
	++size;
}

template<typename T> inline const T& Queue<T>::Peek() const { return array[back]; }

template<typename T> inline bool Queue<T>::Pop(T& value)
{
	if (size)
	{
		value = array[back++];
		--size;

		if (back == capacity) { back = 0; }

		return true;
	}

	return false;
}

template<typename T> inline bool Queue<T>::Pop(T&& value) noexcept
{
	if (size)
	{
		value = Move(array[back++]);
		--size;

		if (back == capacity) { back = 0; }

		return true;
	}

	return false;
}

template<typename T> inline void Queue<T>::Reserve(U64 capacity)
{
	Memory::Reallocate(&array, capacity, this->capacity);
}

template<typename T> inline U64 Queue<T>::Capacity() const { return capacity; }

template<typename T> inline U64 Queue<T>::Size() const { return size; }