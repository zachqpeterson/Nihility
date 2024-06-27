module;

#include "Defines.hpp"

#include <atomic>

export module Containers:SafeQueue;

import ThreadSafety;

inline constexpr U64 CacheLineSize = 64;

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

		Node& operator=(const Type& value)
		{
			LockGuard lg(spinLock);
			data = value;

			return *this;
		}

		Node& operator=(Type&& value) noexcept
		{
			LockGuard lg(spinLock);
			data = Move(value);

			return *this;
		}

		void GetData(Type& value)
		{
			LockGuard lg(spinLock);
			value = Move(data);
		}
	};

public:
	SafeQueue() : cursor(Cursor{}) {}

	bool Push(const Type& value)
	{
		Cursor currentCursor;

		while (true)
		{
			currentCursor = cursor.load(std::memory_order_acquire);

			if (currentCursor.producer == currentCursor.consumer + capacity) { return false; }

			if (cursor.compare_exchange_weak(currentCursor, { currentCursor.producer + 1, currentCursor.consumer },
				std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}

			YieldThread();
		}

		buffer[currentCursor.producer & capacityMask] = value;

		return true;
	}

	bool Push(Type&& value) noexcept
	{
		Cursor currentCursor;

		while (true)
		{
			currentCursor = cursor.load(std::memory_order_acquire);

			if (currentCursor.producer == currentCursor.consumer + capacity) { return false; }

			if (cursor.compare_exchange_weak(currentCursor, { currentCursor.producer + 1, currentCursor.consumer },
				std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}

			YieldThread();
		}

		buffer[currentCursor.producer & capacityMask] = Move(value);

		return true;
	}

	bool Pop(Type& value)
	{
		Cursor currentCursor;

		while (true)
		{
			currentCursor = cursor.load(std::memory_order_acquire);

			if (currentCursor.consumer == currentCursor.producer) { return false; }

			if (cursor.compare_exchange_weak(currentCursor, { currentCursor.producer, currentCursor.consumer + 1 },
				std::memory_order_release, std::memory_order_relaxed))
			{
				break;
			}

			YieldThread();
		}

		buffer[currentCursor.consumer & capacityMask].GetData(value);

		return true;
	}

	U32 Size() const
	{
		const Cursor cursors = cursor.load(std::memory_order_acquire);
		return cursors.producer - cursors.consumer;
	}

	bool Empty() const { return Size() == 0; }

	bool Full() const { return Size() == capacity; }

private:
	static constexpr inline U32 capacity = BitCeiling(Capacity);
	static constexpr inline U32 capacityMask = capacity - 1;
	Node buffer[capacity];

	alignas(CacheLineSize) std::atomic<Cursor> cursor;

private:
	SafeQueue(const SafeQueue&) = delete;
	SafeQueue& operator=(const SafeQueue&) = delete;
};