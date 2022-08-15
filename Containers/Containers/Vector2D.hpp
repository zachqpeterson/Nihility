#pragma once

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"
#include "Math/Math.hpp"

template<typename T>
struct Vector2D
{
private:
	struct Node
	{
		Node() : array{ nullptr }, size{ 0 }, capacity{ 0 } {}
		void Destroy()
		{
			Memory::Free(array, capacity * sizeof(T), MEMORY_TAG_DATA_STRUCT);
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

		T& operator[](U64 i) { return array[i]; }
		const T& operator[](U64 i) const { return array[i]; }

		T* array;
		U64 size;
		U64 capacity;
	};

public:
	Vector2D() : nodeArray{ nullptr }, size{ 0 }, capacity{ 0 } {}
	~Vector2D()
	{
		for (U64 i = 0; i < capacity; ++i)
		{
			nodeArray[i].Destroy();
		}

		Memory::Free(nodeArray, capacity * sizeof(Node), MEMORY_TAG_DATA_STRUCT);
	}

	void Expand(U64 amt)
	{
		if (size + amt - 1 == capacity)
		{
			Reserve((size + amt) * 2);
		}

		size += amt;
	}

	void ExpandAll(U64 amt)
	{
		U64 newSize = size + amt;

		if (newSize - 1 == capacity)
		{
			Reserve((newSize) * 2);
		}

		size = newSize;

		for (U64 i = 0; i < size; ++i)
		{
			nodeArray[i].Resize(nodeArray[i].size + amt);
		}
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
		if (nodeArray)
		{
			Node* newArray = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);

			Memory::Copy(newArray, nodeArray, sizeof(Node) * (capacity < this->capacity ? capacity : this->capacity));

			Memory::Free(nodeArray, sizeof(Node) * this->capacity, MEMORY_TAG_DATA_STRUCT);
			nodeArray = newArray;
			this->capacity = capacity;
			size = size > capacity ? capacity : size;
		}
		else
		{
			nodeArray = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);
			this->capacity = capacity;
			size = 0;
		}
	}

	T& operator[](const Vector2Int& v) { return nodeArray[v.x][v.y]; }
	const T& operator[](const Vector2Int& v) const { return nodeArray[v.x][v.y]; }

	U64 Size() const { return size; }
	U64 Capacity() const { return capacity; }

private:
	Node* nodeArray;
	U64 size;
	U64 capacity;
};