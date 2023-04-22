#pragma once

#include "Defines.hpp"

#include <condition_variable>

template <typename T, U64 capacity>
class SafeQueue
{
public:
	bool Push(const T& item)
	{
		bool result = false;
		lock.lock();
		U64 next = head + 1;
		if (next == capacity) { next -= capacity; }
		if (next != tail)
		{
			data[head] = item;
			head = next;
			result = true;
		}
		lock.unlock();
		return result;
	}

	bool Pop(T& item)
	{
		bool result = false;
		lock.lock();
		if (tail != head)
		{
			item = data[tail];
			if (++tail == capacity) { tail -= capacity; }
			result = true;
		}
		lock.unlock();
		return result;
	}

private:
	T data[capacity];
	U64 head = 0;
	U64 tail = 0;
	std::mutex lock;
};