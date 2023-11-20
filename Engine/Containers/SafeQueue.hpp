#pragma once

#include "ContainerDefines.hpp"

#include "Memory\Memory.hpp"
#include "Platform\ThreadSafety.hpp"

template<class Type>
struct SafeQueue
{
public:
	SafeQueue();
	SafeQueue(U32 capacity);

	~SafeQueue();
	void Destroy();

	bool Push(const Type& value);
	bool Push(Type&& value) noexcept;
	const Type& Peek() const;
	bool Pop(Type& value);

	bool Full() const;
	bool Empty() const;

private:
	U32 size{ 0 };
	U32 capacity{ 0 };

	U32 front{ 0 };
	U32 back{ 0 };
	Type* array{ nullptr };
};

template<class Type>
inline SafeQueue<Type>::SafeQueue() {}

template<class Type>
inline SafeQueue<Type>::SafeQueue(U32 cap)
{
	Memory::AllocateArray(&array, cap, capacity);
}

template<class Type>
inline SafeQueue<Type>::~SafeQueue()
{
	Destroy();
}

template<class Type>
inline void SafeQueue<Type>::Destroy()
{
	front = 0;
	back = 0;
	size = 0;
	capacity = 0;
	if (array) { Memory::Free(&array); }
}

template<class Type>
inline bool SafeQueue<Type>::Push(const Type& value)
{
	if (size == capacity) { return false; }

	array[SafeIncrement(&front) - 1] = value;
	++size;

	return true;
}

template<class Type>
inline bool SafeQueue<Type>::Push(Type&& value) noexcept
{
	if (size == capacity) { return false; }

	array[SafeIncrement(&front) - 1] = Move(value);
	++size;

	return true;
}

template<class Type>
inline const Type& SafeQueue<Type>::Peek() const
{
	return array[back];
}

template<class Type>
inline bool SafeQueue<Type>::Pop(Type& value)
{
	if (SafeDecrement(&size) < capacity)
	{
		U32 b = SafeCompareAndExchange(&back, 0U, capacity);

		if (b == back) { value = Move(array[SafeIncrement(&back) - 1]); return true; }

		value = Move(array[0]);
		return true;
	}

	++size;

	return false;
}

template<class Type>
inline bool SafeQueue<Type>::Full() const
{
	return size == capacity;
}

template<class Type>
inline bool SafeQueue<Type>::Empty() const
{
	return size == 0;
}
