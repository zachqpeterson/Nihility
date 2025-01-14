#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Memory\Memory.hpp"

template<class Type>
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
	void Clear();

	void Push(const Type& value);
	void Push(Type&& value) noexcept;
	bool Pop(Type& value);

	void Reserve(U64 capacity);

	U64 Capacity() const;
	U64 Size() const;
	bool Empty() const;
	bool Full() const;

private:
	U64 capacity = 0;
	U64 capacityMask = 0;
	U64 front = 0;
	U64 back = 0;
	Type* array = nullptr;
};

template<class Type> 
inline Queue<Type>::Queue()
{
	Memory::AllocateArray(&array, capacity, capacity);
	capacity = BitFloor(capacity);
	capacityMask = capacity - 1;
}

template<class Type>
inline Queue<Type>::Queue(U64 cap) : capacity(BitCeiling(cap))
{
	Memory::AllocateArray(&array, capacity, capacity);
	capacity = BitFloor(capacity);
	capacityMask = capacity - 1;
}

template<class Type>
inline Queue<Type>::Queue(const Queue<Type>& other) : capacity(other.capacity), capacityMask(other.capacity), front(other.front), back(other.back)
{
	Memory::AllocateArray(&array, capacity);
	Copy(array, other.array, capacity);
}

template<class Type>
inline Queue<Type>::Queue(Queue<Type>&& other) : capacity(other.capacity), capacityMask(other.capacity), front(other.front), back(other.back), array(other.array)
{
	other.array = nullptr;
	other.Destroy();
}

template<class Type>
inline Queue<Type>& Queue<Type>::operator=(const Queue<Type>& other)
{
	if (array) { Memory::Free(array); }
	front = other.front;
	back = other.back;
	capacity = other.capacity;
	capacityMask = other.capacityMask;
	Memory::AllocateArray(&array, capacity);

	Copy(array, other.array, capacity);

	return *this;
}

template<class Type>
inline Queue<Type>& Queue<Type>::operator=(Queue<Type>&& other)
{
	if (array) { Memory::Free(array); }
	front = other.front;
	back = other.back;
	capacity = other.capacity;
	capacityMask = other.capacityMask;
	array = other.array;

	other.array = nullptr;
	other.Destroy();

	return *this;
}

template<class Type>
inline Queue<Type>::~Queue() { Destroy(); }

template<class Type>
inline void Queue<Type>::Destroy()
{
	front = 0;
	back = 0;
	capacity = 0;
	capacityMask = 0;
	if (array) { Memory::Free(&array); }
}

template<class Type>
inline void Queue<Type>::Clear()
{
	front = 0;
	back = 0;
}

template<class Type>
inline void Queue<Type>::Push(const Type& value)
{
	if (Full()) { Reserve(capacity + 1); }

	array[front++] = value;
}

template<class Type>
inline void Queue<Type>::Push(Type&& value) noexcept
{
	if (Full()) { Reserve(capacity + 1); }

	array[front++] = Move(value);
}

template<class Type>
inline bool Queue<Type>::Pop(Type& value)
{
	if (Empty()) { return false; }

	value = Move(array[back++ & capacityMask]);

	return true;
}

template<class Type>
inline void Queue<Type>::Reserve(U64 capacity)
{
	capacity = BitCeiling(capacity);
	Memory::Reallocate(&array, capacity, this->capacity);
	capacity = BitFloor(capacity);
	capacityMask = capacity - 1;
}

template<class Type>
inline U64 Queue<Type>::Capacity() const { return capacity; }

template<class Type>
inline U64 Queue<Type>::Size() const { return front - back; }

template<class Type>
inline bool Queue<Type>::Empty() const { return front == back; }

template<class Type>
inline bool Queue<Type>::Full() const { return front == back + capacity; }