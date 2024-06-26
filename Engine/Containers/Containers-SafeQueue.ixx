module;

#include "Defines.hpp"

export module Containers:SafeQueue;

import ThreadSafety;

export template<CopyOrMoveable Type, U64 Capacity>
struct SafeQueue
{
	SafeQueue() {}

	~SafeQueue() { Destroy(); }
	void Destroy();

	bool Push(const Type& value);
	bool Push(Type&& value) noexcept;
	bool Pop(Type& value);

	bool Full() const;
	bool Empty() const;

private:
	static constexpr inline U64 CapacityMask = Capacity - 1;

	U64 front{ 0 };
	U64 back{ 0 };

	Type array[Capacity];
};

template<CopyOrMoveable Type, U64 Capacity>
inline void SafeQueue<Type, Capacity>::Destroy()
{
	front = 0;
	back = 0;
}

template<CopyOrMoveable Type, U64 Capacity>
inline bool SafeQueue<Type, Capacity>::Push(const Type& value)
{
	U64 index = SafeIncrement(&front) - 1;
	if (Full())
	{
		--front;
		return false;
	}

	array[index & CapacityMask] = value;

	return true;
}

template<CopyOrMoveable Type, U64 Capacity>
inline bool SafeQueue<Type, Capacity>::Push(Type&& value) noexcept
{
	U64 index = SafeIncrement(&front) - 1;
	if (Full())
	{
		--front;
		return false;
	}

	array[index & CapacityMask] = Move(value);

	return true;
}

template<CopyOrMoveable Type, U64 Capacity>
inline bool SafeQueue<Type, Capacity>::Pop(Type& value)
{
	U64 index = SafeIncrement(&back) - 1;
	if (Empty())
	{
		--back;
		return false;
	}

	value = Move(array[index & CapacityMask]);
	return true;
}

template<CopyOrMoveable Type, U64 Capacity>
inline bool SafeQueue<Type, Capacity>::Full() const
{
	return ((front + 1) & CapacityMask) == (back & CapacityMask);
}

template<CopyOrMoveable Type, U64 Capacity>
inline bool SafeQueue<Type, Capacity>::Empty() const
{
	return front <= back;
}

//#include "stdio.h"
//#include "stdlib.h"
//
//#include <atomic>
//#include <stdint.h>
//#include <functional>
//#include <thread>
//
//#if defined(_MSC_VER)
//#define _ENABLE_ATOMIC_ALIGNMENT_FIX    1 // MSVC atomic alignment fix.
//#define ATOMIC_ALIGNMENT                4
//#else
//#define ATOMIC_ALIGNMENT                16
//#if defined(__clang__) || defined(__GNUC__)
//#endif
//#endif
//
//inline constexpr U64 CacheLineSize = std::hardware_destructive_interference_size;
//
//template <CopyOrMoveable Type, U32 Capacity>
//class SafeQueue final
//{
//	/**
//	 * Structure that holds the two cursors.
//	 * The cursors are held together because we'll only ever be accessing
//	 * them both at the same time.
//	 * We don't directly align the struct because we need to use it as an
//	 * atomic variable, so we must align the atomic variable instead.
//	 */
//	struct cursor_data
//	{
//		U32 producer_cursor;
//		U32 consumer_cursor;
//
//		cursor_data(const U32 in_producer_cursor = 0,
//			const U32 in_consumer_cursor = 0)
//			: producer_cursor(in_producer_cursor),
//			consumer_cursor(in_consumer_cursor)
//		{
//		}
//	};
//
//	/**
//	 * Structure that represents each node in the circular buffer.
//	 * Access to the data is protected by a spin lock.
//	 * Contention on the spin lock should be minimal, as it's only there
//	 * to prevent the case where a producer/consumer may try work with an element before
//	 * someone else has finished working with it. The data and the spin lock are seperated by
//	 * padding to put them in differnet cache lines, since they are not accessed
//	 * together in the case mentioned previously. The problem with this is
//	 * that in low contention cases, they will be accessed together, and thus
//	 * should be in the same cache line. More testing is needed here.
//	 */
//	struct buffer_node
//	{
//		alignas(CacheLineSize) Type data;
//		alignas(CacheLineSize) SpinLock spinLock;
//
//		void get_data(Type& out_data) const
//		{
//			LockGuard lg(spinLock);
//			out_data = data;
//		}
//
//		void set_data(const T& in_data)
//		{
//			LockGuard lg(spinLock);
//			data = in_data;
//		}
//	};
//
//public:
//	SafeQueue() : cursor_data_(cursor_data{})
//	{
//	}
//
//	bool push(const Type& in_data)
//	{
//		cursor_data current_cursor_data;
//
//		// An infinite while-loop is used instead of a do-while, to avoid
//		// the yield/pause happening before the CAS operation.
//		while (true)
//		{
//			current_cursor_data = cursor_data_.load(std::memory_order_acquire);
//
//			// Check if the buffer is full..
//			if ((current_cursor_data.producer_cursor + 1) & circular_buffer_data_.index_mask ==
//				current_cursor_data.consumer_cursor & circular_buffer_data_.index_mask)
//			{
//				return false;
//			}
//
//			// CAS operation used to make sure the cursors have not been incremented
//			// by another producer/consumer before we got to this point, and to then increment
//			// the cursor by 1 if it hasn't been changed.
//			if (cursor_data_.compare_exchange_weak(current_cursor_data,
//				{ current_cursor_data.producer_cursor + 1,
//					current_cursor_data.consumer_cursor },
//				std::memory_order_release, std::memory_order_relaxed))
//			{
//				break;
//			}
//
//			should_yield_not_pause ? std::this_thread::yield() : HARDWARE_PAUSE();
//		}
//
//		// Set the data
//		circular_buffer_data_.circular_buffer[
//			current_cursor_data.producer_cursor & circular_buffer_data_.index_mask
//		].set_data(in_data);
//
//		return true;
//	}
//
//	bool pop(Type& out_data)
//	{
//		cursor_data current_cursor_data;
//
//		while (true)
//		{
//			current_cursor_data = cursor_data_.load(std::memory_order_acquire);
//
//			// Check if the queue is empty.. 
//			if (current_cursor_data.consumer_cursor == current_cursor_data.producer_cursor)
//			{
//				return false;
//			}
//
//			if (cursor_data_.compare_exchange_weak(current_cursor_data,
//				{ current_cursor_data.producer_cursor,
//					current_cursor_data.consumer_cursor + 1 },
//				std::memory_order_release, std::memory_order_relaxed))
//			{
//				break;
//			}
//
//			should_yield_not_pause ? std::this_thread::yield() : HARDWARE_PAUSE();
//		}
//
//		// Get the data
//		circular_buffer_data_.circular_buffer[
//			current_cursor_data.consumer_cursor & circular_buffer_data_.index_mask
//		].get_data(out_data);
//
//		return true;
//	}
//
//	U32 size() const
//	{
//		const cursor_data cursors = cursor_data_.load(std::memory_order_acquire);
//		return cursors.producer_cursor - cursors.consumer_cursor;
//	}
//
//	bool empty() const
//	{
//		return size() == 0;
//	}
//
//	bool full() const
//	{
//		return size() == circular_buffer_data_.index_mask + 1;
//	}
//
//private:
//	constexpr U32 capacity = BitCeiling(Capacity);
//	buffer_node circular_buffer[capacity + 1];
//
//	alignas(CacheLineSize) std::atomic<cursor_data> cursor_data_;
//
//private:
//	bounded_circular_mpmc_queue(
//		const bounded_circular_mpmc_queue&) = delete;
//	bounded_circular_mpmc_queue& operator=(
//		const bounded_circular_mpmc_queue&) = delete;
//};