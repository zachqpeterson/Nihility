#pragma once

#include "Defines.hpp"

template<typename T>
struct Queue
{
public:
	Queue();
	Queue(U64 capacity);

	~Queue();
	void Destroy();

	void Push(const T& value);
	void Push(T&& value) noexcept;
	const T& Peek() const;
	void Pop(T& value);
	void Pop(T&& value) noexcept;

	const U64& Capacity() const;
	const U64& Size() const;

private:

	U64 capacity;
	U64 size;
	T* array;

	Queue(const Queue&) = delete;
	Queue(Queue&&) = delete;
	Queue& operator=(const Queue&) = delete;
	Queue& operator=(Queue&&) = delete;
};