#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

//TODO: Use quadratic probing
template <typename TKey, typename TValue>
struct HashTable
{
public:
	struct Node
	{
		bool filled{ false };
		TKey key;
		TValue value;
	};

	struct Iterator
	{
		Iterator(Node* ptr, Node* memory, U64 capacity) : ptr{ ptr }, memory{ memory }, capacity{ capacity } 
		{
			Node* end = &memory[capacity];
			while (!this->ptr->filled && (this->ptr != end)) { ++this->ptr; }
		}

		Node& operator* () const { return *ptr; }
		Node* operator-> () { return ptr; }

		Iterator& operator++ ()
		{
			Node* end = &memory[capacity];
			while (!(++ptr)->filled && (ptr != end));
			return *this;
		}
		Iterator operator++ (int)
		{
			Node* end = &memory[capacity];
			Iterator temp = *this;
			while (!(++ptr)->filled && (ptr != end));
			return temp;
		}

		Iterator& operator-- ()
		{
			Node* end = memory - 1;
			while (!(--ptr)->filled && (ptr != end));

			return *this;
		}
		Iterator operator-- (int)
		{
			Node* end = memory - 1;
			Iterator temp = *this;
			while (!(--ptr)->filled && (ptr != end));
			return temp;
		}

		Iterator operator+(int i)
		{
			Node* end = &memory[capacity];
			Iterator temp = *this;

			U64 j = 0;
			while (j < i) { j += (++ptr)->filled + i * (ptr == end); }

			return temp;
		}

		Iterator operator-(int i)
		{
			Node* end = memory - 1;
			Iterator temp = *this;
			U64 j = 0;
			while (j < i) { j += (--ptr)->filled + i * (ptr == end); }
			return temp;
		}

		Iterator& operator+=(int i)
		{
			Node* end = &memory[capacity];
			U64 j = 0;
			while (j < i) { j += (++ptr)->filled + i * (ptr == end); }
			return *this;
		}

		Iterator& operator-=(int i)
		{
			Node* end = memory - 1;
			U64 j = 0;
			while (j < i) { j += (--ptr)->filled + i * (ptr == end); }
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) 
		{ 
			return a.ptr != b.ptr; 
		}
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

		operator bool() { return ptr; }

	private:
		Node* ptr;
		Node* memory;
		U64 capacity;
	};

public:
	HashTable() : size{ 0 }, capacity{ 0 }, memory{ nullptr } {}

	HashTable(U64 capacity) : size{ 0 }, capacity{ capacity }, memory{ (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT) } {}

	HashTable(const HashTable& other) : size{ other.size }, capacity{ other.capacity }, memory{ (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT) }
	{
		Memory::Copy(other.memory, memory, sizeof(Node) * capacity);
	}

	HashTable(HashTable&& other) : size{ other.size }, capacity{ other.capacity }, memory{ other.memory }
	{
		other.size = 0;
		other.capacity = 0;
		other.memory = nullptr;
	}

	~HashTable() { Destroy(); }

	void Destroy()
	{
		size = 0;
		if (memory) { Memory::Free(memory, sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT); memory = nullptr; }
		capacity = 0;
	}

	void* operator new(U64 size) { return Memory::Allocate(sizeof(HashTable), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(HashTable), MEMORY_TAG_DATA_STRUCT); }

	HashTable& operator=(const HashTable& other)
	{
		size = other.size;
		capacity = other.capacity;
		memory = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);
		Memory::Copy(other.memory, memory, sizeof(Node) * capacity);
	}

	HashTable& operator=(HashTable&& other)
	{
		size = other.size;
		capacity = other.capacity;
		memory = other.memory;

		other.size = 0;
		other.capacity = 0;
		other.memory = nullptr;
	}

	void Insert(const TKey& key, const TValue& value)
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		++size;
		node->filled = true;
		node->value = value;
		node->key = key;
	}

	void Remove(const TKey& key, TValue&& value)
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->key != key && node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		--size;
		node->filled = false;
		value = Move(node->value);
		Memory::Zero(node, sizeof(Node));
	}

	void Empty()
	{
		Memory::Zero(memory, sizeof(Node) * capacity);
		size = 0;
	}

	TValue& Get(const TKey& key)
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->key != key && node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		return node->value;
	}

	const TValue& Get(const TKey& key) const
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->key != key && node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		return node->value;
	}

	TValue& operator[](const TKey& key)
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->key != key && node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		return node->value;
	}

	const TValue& operator[](const TKey& key) const
	{
		U64 hash = Math::Hash(key);

		U32 i = 0;
		Node* node = memory + (hash % capacity);
		while (node->key != key && node->filled) { ++i; node = &memory[(hash + i * i) % capacity]; }

		return node->value;
	}

	void Reserve(U64 capacity)
	{
		if (capacity < this->capacity) { return; }

		if (memory)
		{
			Node* newArray = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);

			Memory::Copy(newArray, memory, sizeof(Node) * capacity);

			Memory::Free(memory, sizeof(Node) * this->capacity, MEMORY_TAG_DATA_STRUCT);
			memory = newArray;
			this->capacity = capacity;
		}
		else
		{
			memory = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);
			this->capacity = capacity;
			size = 0;
		}
	}

	void operator()(U64 capacity)
	{
		if (capacity < this->capacity) { return; }

		if (memory)
		{
			Node* newArray = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);

			Memory::Copy(newArray, memory, sizeof(Node) * capacity);

			Memory::Free(memory, sizeof(Node) * this->capacity, MEMORY_TAG_DATA_STRUCT);
			memory = newArray;
			this->capacity = capacity;
		}
		else
		{
			memory = (Node*)Memory::Allocate(sizeof(Node) * capacity, MEMORY_TAG_DATA_STRUCT);
			this->capacity = capacity;
			size = 0;
		}
	}

	Iterator begin() { return Iterator(memory, memory, capacity); }
	Iterator end() { return Iterator(&memory[capacity], memory, capacity); }
	Iterator begin() const { return Iterator(memory, memory, capacity); }
	Iterator end() const { return Iterator(&memory[capacity], memory, capacity); }

private:
	U64 size;
	U64 capacity;
	Node* memory;
};