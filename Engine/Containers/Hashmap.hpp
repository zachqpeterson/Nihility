#pragma once

#include "ContainerDefines.hpp"
#include "String.hpp"
#include "Memory\Memory.hpp"

//TODO: U64 key version
template<typename Value>
struct NH_API Hashmap
{
	struct Cell
	{
		bool filled{ false };
		String key{ NO_INIT };
		Value value;
	};

public:
	Hashmap();
	Hashmap(U64 capacity);
	Hashmap(const Hashmap& other);
	Hashmap(Hashmap&& other) noexcept;
	Hashmap& operator=(const Hashmap& other);
	Hashmap& operator=(Hashmap&& other) noexcept;

	~Hashmap();
	void Destroy();

	bool Insert(String& key, const Value& value);
	bool Insert(String& key, Value&& value) noexcept;
	bool Remove(String& key);
	bool Remove(String& key, Value&& value) noexcept;
	bool Get(String& key, Value& value);

	void Reserve(U64 capacity);
	void operator()(U64 capacity);
	void Empty();

	const U64& Size() const;
	const U64& Capacity() const;

private:
	Cell* begin() { return cells; }
	const Cell* begin() const { return cells; }
	Cell* end() { return cells + capacity; }
	const Cell* end() const { return cells + capacity; }

	U64 size{ 0 };
	U64 capacity{ 0 };
	U64 capMinusOne{ 0 };
	Cell* cells{ nullptr };
};

template<typename Value> inline Hashmap<Value>::Hashmap() {}

template<typename Value> inline Hashmap<Value>::Hashmap(U64 cap) : capacity{ capacity } { Memory::AllocateArray(&cells, capacity); capMinusOne = capacity - 1; }

template<typename Value> inline Hashmap<Value>::Hashmap(const Hashmap& other) : size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	Memory::AllocateArray(&cells, capacity);
	memcpy(other.cells, cells, capacity * sizeof(Cell));
}

template<typename Value> inline Hashmap<Value>::Hashmap(Hashmap&& other) noexcept :
	cells{ other.cells }, size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	other.cells = nullptr;
	other.size = 0;
	other.capacity = 0;
	other.capMinusOne = 0;
}

template<typename Value> inline Hashmap<Value>& Hashmap<Value>::operator=(const Hashmap& other)
{
	size = other.size;
	capacity = other.capacity;
	Memory::AllocateArray(&cells, capacity);
	capMinusOne = other.capMinusOne;
	memcpy(other.cells, cells, capacity * sizeof(Cell));

	return *this;
}

template<typename Value> inline Hashmap<Value>& Hashmap<Value>::operator=(Hashmap&& other) noexcept
{
	cells = other.cells;
	size = other.size;
	capacity = other.capacity;
	capMinusOne = other.capMinusOne;

	other.cells = nullptr;
	other.size = 0;
	other.capacity = 0;
	other.capMinusOne = 0;

	return *this;
}

template<typename Value> inline Hashmap<Value>::~Hashmap() { Destroy(); }

template<typename Value> inline void Hashmap<Value>::Destroy()
{
	for (Cell& cell : *this) { cell.key.Destroy(); }

	Memory::FreeArray(&cells);
	size = 0;
	capacity = 0;
	capMinusOne = 0;
}

template<typename Value> inline bool Hashmap<Value>::Insert(String& key, const Value& value)
{
	if (size == capacity) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = value;
	cell->key = key;
}

template<typename Value> inline bool Hashmap<Value>::Insert(String& key, Value&& value) noexcept
{
	if (size == capacity) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = Move(value);
	cell->key = key;
}

template<typename Value> inline bool Hashmap<Value>::Remove(String& key)
{
	if (size == 0) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;
		cell->key.Destroy();
		memset(cell, 0, sizeof(cell));

		return true;
	}

	return false;
}

template<typename Value> inline bool Hashmap<Value>::Remove(String& key, Value&& value) noexcept
{
	if (size == 0) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;

		value = Move(cell->value);

		cell->key.Destroy();
		memset(cell, 0, sizeof(cell));

		return true;
	}

	return false;
}

template<typename Value> inline bool Hashmap<Value>::Get(String& key, Value& value)
{
	if (size == 0) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell* cell = cells + (hash % capacity);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	value = cell->value;

	return cell->filled;
}

template<typename Value> inline void Hashmap<Value>::Reserve(U64 capacity)
{
	if (capacity < this->capacity) { return; }

	this->capacity = capacity;
	Memory::Reallocate(&cells, this->capacity);
	capMinusOne = this->capacity - 1;

	Empty();
}

template<typename Value> inline void Hashmap<Value>::operator()(U64 capacity) { Reserve(capacity); }

template<typename Value> inline void Hashmap<Value>::Empty()
{
	for (Cell& cell : *this) { cell.key.Destroy(); }

	memset(cells, 0, capacity * sizeof(Cell));
	size = 0;
}

template<typename Value> inline const U64& Hashmap<Value>::Size() const { return size; }
template<typename Value> inline const U64& Hashmap<Value>::Capacity() const { return size; }
