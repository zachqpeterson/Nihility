module;

#include "Defines.hpp"

#include "stdio.h"
#include "stdlib.h"

#include <atomic>
#include <stdint.h>
#include <functional>
#include <thread>

export module Containers:SafeQueue;

import ThreadSafety;

//export template<CopyOrMoveable Type, U64 Size>
//struct SafeQueue
//{
//	SafeQueue() {}
//
//	~SafeQueue() { Destroy(); }
//	void Destroy();
//
//	bool Push(const Type& value);
//	bool Push(Type&& value) noexcept;
//	bool Pop(Type& value);
//
//	bool Full() const;
//	bool Empty() const;
//
//private:
//	static constexpr inline U64 Capacity = BitCeiling(Size);
//	static constexpr inline U64 CapacityMask = Capacity - 1;
//
//	U64 front{ 0 };
//	U64 back{ 0 };
//
//	Type array[Capacity];
//};
//
//template<CopyOrMoveable Type, U64 Size>
//inline void SafeQueue<Type, Size>::Destroy()
//{
//	front = 0;
//	back = 0;
//}
//
//template<CopyOrMoveable Type, U64 Size>
//inline bool SafeQueue<Type, Size>::Push(const Type& value)
//{
//	U64 index = SafeIncrement(&front) - 1;
//	if (Full())
//	{
//		--front;
//		return false;
//	}
//
//	array[index & CapacityMask] = value;
//
//	return true;
//}
//
//template<CopyOrMoveable Type, U64 Size>
//inline bool SafeQueue<Type, Size>::Push(Type&& value) noexcept
//{
//	U64 index = SafeIncrement(&front) - 1;
//	if (Full())
//	{
//		--front;
//		return false;
//	}
//
//	array[index & CapacityMask] = Move(value);
//
//	return true;
//}
//
//template<CopyOrMoveable Type, U64 Size>
//inline bool SafeQueue<Type, Size>::Pop(Type& value)
//{
//	U64 index = SafeIncrement(&back) - 1;
//	if (Empty())
//	{
//		--back;
//		return false;
//	}
//
//	value = Move(array[index & CapacityMask]);
//	return true;
//}
//
//template<CopyOrMoveable Type, U64 Size>
//inline bool SafeQueue<Type, Size>::Full() const
//{
//	return front >= back + Capacity;
//}
//
//template<CopyOrMoveable Type, U64 Size>
//inline bool SafeQueue<Type, Size>::Empty() const
//{
//	return front <= back;
//}

inline constexpr U64 CacheLineSize = std::hardware_destructive_interference_size;

export template <CopyOrMoveable Type, U32 Capacity>
struct SafeQueue
{
	struct Cursor
	{
		U32 producer = 0;
		U32 consumer = 0;
	};

	struct Node
	{
		alignas(CacheLineSize) Type data;
		alignas(CacheLineSize) SpinLock spinLock;

		void GetData(Type& out) const
		{
			LockGuard lg(spinLock);
			out = data;
		}

		void SetData(const Type& in)
		{
			LockGuard lg(spinLock);
			data = in;
		}
	};

public:
	SafeQueue() : cursor(Cursor{}) {}

	bool Push(const Type& in_data)
	{
		Cursor currentCursor;

		while (true)
		{
			currentCursor = cursor.load(std::memory_order_acquire);

			if (((currentCursor.producer + 1) & capacityMask) == (currentCursor.consumer & capacityMask))
			{
				return false;
			}

			if (cursor.compare_exchange_weak(currentCursor, { currentCursor.producer + 1, currentCursor.consumer },
				std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}

			YieldThread();
		}

		buffer[currentCursor.producer & capacityMask].SetData(in_data);

		return true;
	}

	bool Pop(Type& out_data)
	{
		Cursor currentCursor;

		while (true)
		{
			currentCursor = cursor.load(std::memory_order_acquire);

			if (currentCursor.consumer == currentCursor.producer)
			{
				return false;
			}

			if (cursor.compare_exchange_weak(currentCursor, { currentCursor.producer, currentCursor.consumer + 1 },
				std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}

			YieldThread();
		}

		buffer[currentCursor.consumer & capacityMask].GetData(out_data);

		return true;
	}

	U32 Size() const
	{
		const Cursor cursors = cursor.load(std::memory_order_acquire);
		return cursors.producer - cursors.consumer;
	}

	bool Empty() const
	{
		return Size() == 0;
	}

	bool Full() const
	{
		return Size() == capacity;
	}

private:
	static constexpr inline U32 capacity = BitCeiling(Capacity);
	static constexpr inline U32 capacityMask = capacity - 1;
	Node buffer[capacity];

	alignas(CacheLineSize) std::atomic<Cursor> cursor;

private:
	SafeQueue(const SafeQueue&) = delete;
	SafeQueue& operator=(const SafeQueue&) = delete;
};