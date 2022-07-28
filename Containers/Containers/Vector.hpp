#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

//TODO: Handle Pointer types
//template<typename T>
//struct Vector<T*>

template<typename T>
struct Vector
{
public:
	struct Iterator
	{
		Iterator(T* ptr) : ptr{ ptr } {}

		T& operator* () const { return *ptr; }
		T* operator-> () { return ptr; }

		Iterator& operator++ () { ++ptr; return *this; }
		Iterator operator++ (int)
		{
			Iterator temp = *this;
			++ptr;
			return temp;
		}

		Iterator& operator-- () { --ptr; return *this; }
		Iterator operator-- (int)
		{
			Iterator temp = *this;
			--ptr;
			return temp;
		}

		Iterator operator+(int i)
		{
			Iterator temp = *this;
			temp += i;
			return temp;
		}

		Iterator operator-(int i)
		{
			Iterator temp = *this;
			temp -= i;
			return temp;
		}

		Iterator& operator+=(int i)
		{
			ptr += i;
			return *this;
		}

		Iterator& operator-=(int i)
		{
			ptr -= i;
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

		operator bool() { return ptr; }

	private:
		T* ptr;
	};

public:
	Vector() : size{ 0 }, capacity{ 0 }, array{ nullptr } {}
	Vector(U64 capacity) : size{ 0 }, capacity{ capacity }, array{ (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT) } {}
	Vector(U64 size, const T& value) : size{ size }, capacity{ size }
	{
		array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

		for (U64 i = 0; i < size; ++i)
		{
			array[i] = value;
		}
	}
	Vector(const Vector& other) : size{ other.size }, capacity{ other.capacity }
	{
		array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

		Memory::Copy(array, other.array, sizeof(T) * size);
	}
	Vector(Vector&& other) noexcept : size{ other.size }, capacity{ other.capacity }, array{ other.array }
	{
		other.size = 0;
		other.capacity = 0;
		other.array = nullptr;
	}
	Vector(T* array, U64 size) : array{ array }, size{ size }, capacity{ size } {}
	~Vector()
	{
		if (array)
		{
			Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		array = nullptr;
		size = 0;
		capacity = 0;
	}
	void Destroy()
	{
		if (array)
		{
			Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		array = nullptr;
		size = 0;
		capacity = 0;
	}

	void* operator new(U64 size) { return Memory::Allocate(sizeof(Vector), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Vector), MEMORY_TAG_DATA_STRUCT); }

	Vector& operator=(const Vector& other)
	{
		if (array)
		{
			Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		size = other.size;
		capacity = other.capacity;
		array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

		Memory::Copy(array, other.array, sizeof(T) * size);

		return *this;
	}
	Vector& operator=(Vector&& other) noexcept
	{
		if (array)
		{
			Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		size = other.size;
		capacity = other.capacity;
		array = other.array;
		other.size = 0;
		other.capacity = 0;
		other.array = nullptr;

		return *this;
	}

	void CopyArray(T* array, U64 size)
	{
		if (this->array)
		{
			Memory::Free(this->array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		this->array = (T*)Memory::Allocate(sizeof(T) * size, MEMORY_TAG_DATA_STRUCT);
		Memory::Copy(this->array, array, sizeof(T) * size);
		this->size = size;
		capacity = size;
	}
	void SetArray(T* array, U64 size)
	{
		if (this->array)
		{
			Memory::Free(this->array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
		}

		this->array = array;
		this->size = size;
		capacity = size;
	}

	void Push(const T& value)
	{
		if (size == capacity)
		{
			Reserve((capacity + 1) * 2);
		}

		array[size] = value;
		++size;
	}
	void Push(T&& value)
	{
		if (size == capacity)
		{
			Reserve((capacity + 1) * 2);
		}

		array[size] = value;
		++size;
	}
	T&& Pop()
	{
		ASSERT_DEBUG_MSG(size, "Can't pop on a vector of size 0!");
		return Move(array[--size]);
	}
	void Insert(const T& value, U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");

		if (size == capacity)
		{
			Reserve((capacity + 1) * 2);
		}

		++size;

		if (index < size - 1)
		{
			Memory::Copy(array + index + 1, array + index, sizeof(T) * (size - (index + 1)));
		}

		array[index] = value;
	}
	void Insert(T&& value, U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");

		if (size == capacity)
		{
			Reserve((capacity + 1) * 2);
		}

		++size;

		if (index < size - 1)
		{
			Memory::Copy(array + index + 1, array + index, sizeof(T) * (size - index));
		}

		array[index] = value;
	}
	T&& Remove(U64 index)
	{
		ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");
		T value = array[index];

		Memory::Copy(array + index, array + index + 1, sizeof(T) * (size - index - 1));
		--size;

		return Move(value);
	}
	void Resize(U64 size)
	{
		if (size > capacity)
		{
			Reserve(size);
		}

		this->size = size;
	}
	void Reserve(U64 capacity)
	{
		if (array)
		{
			T* newArray = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

			Memory::Copy(newArray, array, sizeof(T) * (capacity < this->capacity ? capacity : this->capacity));

			Memory::Free(array, sizeof(T) * this->capacity, MEMORY_TAG_DATA_STRUCT);
			array = newArray;
			this->capacity = capacity;
			size = size > capacity ? capacity : size;
		}
		else
		{
			array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
			this->capacity = capacity;
			size = 0;
		}
	}
	const U64 Find(const T& value)
	{
		for (int i = 0; i < size; ++i)
		{
			if (array[size] == value)
			{
				return i;
			}
		}

		return -1;
	}
	const U64 Find(const T& value) const
	{
		for (int i = 0; i < size; ++i)
		{
			if (array[size] == value)
			{
				return i;
			}
		}

		return -1;
	}
	void Clear() { size = 0; } //TODO: Could cause memory leak
	T* Data() { return array; }
	const T* Data() const { return array; }
	T& Front() { return *array; }
	const T& Front() const { return *array; }
	T& Back() { return array[size - 1]; }
	const T& Back() const { return array[size - 1]; }
	const U64& Size() const { return size; }
	const U64& Capacity() const { return capacity; }

	T& operator[](U64 i) { return array[i]; }
	const T& operator[](U64 i) const { return array[i]; }

	Iterator begin() { return Iterator{ array }; }
	Iterator end() { return Iterator{ &array[size] }; }
	Iterator begin() const { return Iterator{ array }; }
	Iterator end() const { return Iterator{ &array[size] }; }

private:
	U64 size;
	U64 capacity;
	T* array;
};