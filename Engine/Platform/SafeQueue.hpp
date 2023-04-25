#pragma once

#include "Defines.hpp"

#include <atomic>
#include <cstdint>

template <typename Type, U64 cacheLineSize = 64>
struct SafeQueue
{
public:
	explicit SafeQueue() : array{}, write { 0 }, read{ 0 }
	{
		for (U32 i = 0; i < 256; ++i)
		{
			array[i].version = i;
		}
	}

	bool Push(const Type& value)
	{
		U8 head = write.load(std::memory_order_relaxed);

		if (array[head].version.load(std::memory_order_acquire) != head) { return false; }
		if (!write.compare_exchange_strong(head, head + 1, std::memory_order_relaxed)) { return false; }

		array[head].value = value;
		array[head].version.store(head + 1, std::memory_order_release);

		return true;
	}

	bool Pop(Type& out)
	{
		U8 tail = read.load(std::memory_order_relaxed);

		if (array[tail].version.load(std::memory_order_acquire) != (tail + 1)) { return false; }
		if (!read.compare_exchange_strong(tail, tail + 1, std::memory_order_relaxed)) { return false; }

		out = array[tail].value;
		array[tail].version.store(tail, std::memory_order_release);

		return true;
	}

private:
	struct alignas(cacheLineSize)Item
	{
		std::atomic<U8> version;
		Type value;
	};

	struct alignas(cacheLineSize)AlignedAtomicU8 : public std::atomic<U8>
	{
		using std::atomic<U8>::atomic;
	};

	Item array[256];

	AlignedAtomicU8 write;
	AlignedAtomicU8 read;

	SafeQueue(const SafeQueue&) = delete;
	SafeQueue(SafeQueue&&) = delete;
	SafeQueue& operator=(const SafeQueue&) = delete;
	SafeQueue& operator=(SafeQueue&&) = delete;
};