#pragma once

#include "Defines.hpp"
#include "String.hpp"
#include "Memory\Memory.hpp"

template<typename Value>
struct Cell
{
	bool filled{ false };
	String key{ NO_INIT };
	Value value;
};

//TODO: U64 key version

//TODO: Align capacity with 2, use binary & instead of modulo
template<typename Value>
struct Hashmap
{
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

	Cell<Value>* begin();
	const Cell<Value>* begin() const;
	Cell<Value>* end();
	const Cell<Value>* end() const;

private:
	Cell<Value>* cells;
	U64 size;
	U64 capacity;
	U64 capMinusOne;
};

template<typename Value> inline Hashmap<Value>::Hashmap() : cells{ nullptr }, size{ 0 }, capacity{ 0 }, capMinusOne{ 0 } {}

template<typename Value> inline Hashmap<Value>::Hashmap(U64 cap) :
	cells{ (Cell<Value>*)Memory::Allocate(sizeof(Cell<Value>) * cap, capacity) }, size{ 0 }, capacity{ Align2(capacity) }, capMinusOne{ capacity - 1 } {}

template<typename Value> inline Hashmap<Value>::Hashmap(const Hashmap& other) :
	cells{ (Cell<Value>*)Memory::Allocate(sizeof(Cell<Value>) * other.capacity) }, size{ other.size }, capacity{ other.capacity }, capMinusOne{other.capMinusOne}
{
	memcpy(other.cells, cells, sizeof(Cell<Value>) * capacity);
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
	cells = (Cell<Value>*)Memory::Allocate(sizeof(Cell<Value>) * other.capacity);
	size = other.size;
	capacity = other.capacity;
	capMinusOne = other.capMinusOne;
	memcpy(other.cells, cells, sizeof(Cell<Value>) * capacity);

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
	//for (Cell<Value>& cell : cells) { cell.key.Destroy(); }

	Memory::Free(cells);
	size = 0;
	capacity = 0;
	capMinusOne = 0;
}

template<typename Value> inline bool Hashmap<Value>::Insert(String& key, const Value& value)
{
	if (size == capacity) { return false; }
	U64 hash = key.Hash();

	U32 i = 0;
	Cell<Value>* cell = cells + (hash & capMinusOne);
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
	Cell<Value>* cell = cells + (hash & capMinusOne);
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
	Cell<Value>* cell = cells + (hash & capMinusOne);
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
	Cell<Value>* cell = cells + (hash & capMinusOne);
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
	Cell<Value>* cell = cells + (hash % capacity);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	value = cell->value;

	return cell->filled;
}

template<typename Value> inline void Hashmap<Value>::Reserve(U64 capacity)
{
	if (capacity < this->capacity) { return; }

	if (cells)
	{
		this->capacity = Align2(capacity);
		Cell<Value>* newCells = (Cell<Value>*)Memory::Allocate(sizeof(Cell<Value>) * this->capacity);

		memcpy(newCells, cells, sizeof(Cell<Value>) * capacity);

		Memory::Free(cells);
		cells = newCells;
	}
	else
	{
		this->capacity = Align2(capacity);
		cells = (Cell<Value>*)Memory::Allocate(sizeof(Cell<Value>) * this->capacity);
		size = 0;
	}
}

template<typename Value> inline void Hashmap<Value>::operator()(U64 capacity) { Reserve(capacity); }

template<typename Value> inline void Hashmap<Value>::Empty()
{
	//for (Cell<Value>& cell : cells) { cell.key.Destroy(); }

	memset(cells, 0, sizeof(Cell) * capacity);
	size = 0;
}

template<typename Value> inline const U64& Hashmap<Value>::Size() const { return size; }
template<typename Value> inline const U64& Hashmap<Value>::Capacity() const { return size; }

template<typename Value> inline Cell<Value>* Hashmap<Value>::begin() { return cells; }
template<typename Value> inline const Cell<Value>* Hashmap<Value>::begin() const { return cells; }
template<typename Value> inline Cell<Value>* Hashmap<Value>::end() { return cells + capacity; }
template<typename Value> inline const Cell<Value>* Hashmap<Value>::end() const { return cells + capacity; }
