#pragma once

#include "ContainerDefines.hpp"
#include "String.hpp"
#include "Memory\Memory.hpp"

static inline U64 Hash(U64 x)
{
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	x = x ^ (x >> 31);
	return x;
}

//TODO: U64 key version
template<class Key, class Value>
struct NH_API Hashmap
{
	struct Cell
	{
		bool filled{ false };
		Key key{ NO_INIT };
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

	bool Insert(Key& key, const Value& value);
	bool Insert(Key& key, Value&& value) noexcept;
	bool Remove(Key& key);
	bool Remove(Key& key, Value&& value) noexcept;
	bool Get(Key& key, Value& value);

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

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap() {}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(U64 cap)
{
	Memory::AllocateArray(&cells, cap, capacity);
	capacity = NextPow2(capacity) >> 1;
	capMinusOne = capacity - 1;
}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(const Hashmap& other) : size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	Memory::AllocateArray(&cells, capacity);
	memcpy(other.cells, cells, capacity * sizeof(Cell));
}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(Hashmap&& other) noexcept :
	cells{ other.cells }, size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	other.cells = nullptr;
	other.size = 0;
	other.capacity = 0;
	other.capMinusOne = 0;
}

template<class Key, class Value> inline Hashmap<Key, Value>& Hashmap<Key, Value>::operator=(const Hashmap& other)
{
	size = other.size;
	capacity = other.capacity;
	Memory::AllocateArray(&cells, capacity);
	capMinusOne = other.capMinusOne;
	memcpy(other.cells, cells, capacity * sizeof(Cell));

	return *this;
}

template<class Key, class Value> inline Hashmap<Key, Value>& Hashmap<Key, Value>::operator=(Hashmap&& other) noexcept
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

template<class Key, class Value> inline Hashmap<Key, Value>::~Hashmap() { Destroy(); }

template<class Key, class Value> inline void Hashmap<Key, Value>::Destroy()
{
	if (cells)
	{
		if constexpr(IsStringType<Key>) { for (Cell& cell : *this) { cell.key.Destroy(); } }

		Memory::FreeArray(&cells);
		size = 0;
		capacity = 0;
		capMinusOne = 0;
	}
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Insert(Key& key, const Value& value)
{
	if (size == capacity) { return false; }

	if constexpr (IsStringType<Key>) { U64 hash = key.Hash(); }
	else if constexpr (IsInteger<Key>) { U64 hash = Hash(key); }
	else { static_assert("Only integers and strings supported for hashing!"); }
	
	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = value;
	cell->key = key;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Insert(Key& key, Value&& value) noexcept
{
	if (size == capacity) { return false; }

	if constexpr (IsStringType<Key>) { U64 hash = key.Hash(); }
	else if constexpr (IsInteger<Key>) { U64 hash = Hash(key); }
	else { static_assert("Only integers and strings supported for hashing!"); }

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = Move(value);
	cell->key = key;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(Key& key)
{
	if (size == 0) { return false; }

	if constexpr (IsStringType<Key>) { U64 hash = key.Hash(); }
	else if constexpr (IsInteger<Key>) { U64 hash = Hash(key); }
	else { static_assert("Only integers and strings supported for hashing!"); }

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;
		if constexpr (IsStringType<Key>) { cell->key.Destroy(); }
		memset(cell, 0, sizeof(cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(Key& key, Value&& value) noexcept
{
	if (size == 0) { return false; }

	if constexpr (IsStringType<Key>) { U64 hash = key.Hash(); }
	else if constexpr (IsInteger<Key>) { U64 hash = Hash(key); }
	else { static_assert("Only integers and strings supported for hashing!"); }

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;

		value = Move(cell->value);

		if constexpr (IsStringType<Key>) { cell->key.Destroy(); }
		memset(cell, 0, sizeof(cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Get(Key& key, Value& value)
{
	if (size == 0) { return false; }

	if constexpr (IsStringType<Key>) { U64 hash = key.Hash(); }
	else if constexpr (IsInteger<Key>) { U64 hash = Hash(key); }
	else { static_assert("Only integers and strings supported for hashing!"); }

	U32 i = 0;
	Cell* cell = cells + (hash % capacity);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	value = cell->value;

	return cell->filled;
}

template<class Key, class Value> inline void Hashmap<Key, Value>::Reserve(U64 cap)
{
	if (cap < capacity) { return; }

	Memory::Reallocate(&cells, cap, capacity);
	capacity = NextPow2(capacity) >> 1;
	capMinusOne = capacity - 1;

	Empty();
}

template<class Key, class Value> inline void Hashmap<Key, Value>::operator()(U64 capacity) { Reserve(capacity); }

template<class Key, class Value> inline void Hashmap<Key, Value>::Empty()
{
	if constexpr (IsStringType<Key>) { for (Cell& cell : *this) { cell.key.Destroy(); } }
	//TODO: Free memory if value is allocated

	memset(cells, 0, capacity * sizeof(Cell));
	size = 0;
}

template<class Key, class Value> inline const U64& Hashmap<Key, Value>::Size() const { return size; }
template<class Key, class Value> inline const U64& Hashmap<Key, Value>::Capacity() const { return size; }
