#pragma once

#include "ContainerDefines.hpp"
#include "String.hpp"
#include "Memory\Memory.hpp"
#include "Math\Hash.hpp"

typedef U64 HashHandle;

template<class Key, class Value>
struct NH_API Hashmap
{
	struct Cell
	{
		bool filled{ false };
		Key key{};
		Value value;
	};

	struct Iterator
	{
	public:
		Iterator(Cell* cell);
		Iterator(const Iterator& other);
		Iterator(Iterator&& other);

		bool Valid() const;

		Value& operator* ();
		Value* operator-> ();

		Iterator operator++();
		Iterator& operator++(int);
		Iterator operator--();
		Iterator& operator--(int);

		operator bool() const;

		bool operator== (const Iterator& other) const;
		bool operator!= (const Iterator& other) const;
		bool operator< (const Iterator& other) const;
		bool operator> (const Iterator& other) const;
		bool operator<= (const Iterator& other) const;
		bool operator>= (const Iterator& other) const;

	private:
		Cell* cell;
	};

public:
	Hashmap();
	Hashmap(U64 capacity);
	Hashmap(U64 capacity, const Value& def);
	Hashmap(const Hashmap& other);
	Hashmap(Hashmap&& other) noexcept;
	Hashmap& operator=(const Hashmap& other);
	Hashmap& operator=(Hashmap&& other) noexcept;

	~Hashmap();
	void Destroy();

	bool Insert(const Key& key, const Value& value);
	bool Insert(const Key& key, Value&& value) noexcept;
	bool Remove(const Key& key);
	bool Remove(const Key& key, Value& value) noexcept;
	Value& Get(const Key& key);
	Value& Request(const Key& key);
	Value& Request(const Key& key, HashHandle& handle);
	HashHandle GetHandle(const Key& key);
	Value& Obtain(HashHandle handle);
	bool Remove(HashHandle handle);
	bool Remove(HashHandle handle, Value& value) noexcept;

	Value& operator[](const Key& key);
	const Value& operator[](const Key& key) const;

	void Reserve(U64 capacity);
	void operator()(U64 capacity);
	void Empty();

	const U64& Size() const;
	const U64& Capacity() const;

	Iterator begin() { return { cells }; }
	const Iterator begin() const { return { cells }; }
	Iterator end() { return { cells + capacity }; }
	const Iterator end() const { return { cells + capacity }; }

private:
	U64 size{ 0 };
	U64 capacity{ 0 };
	U64 capMinusOne{ 0 };
	Cell* cells{ nullptr };
	Value defVal;
};

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap() {}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(U64 cap)
{
	Memory::AllocateArray(&cells, cap, capacity);
	capacity = BitCeiling(capacity) >> 1;
	capMinusOne = capacity - 1;
	defVal = {};
}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(U64 cap, const Value& def)
{
	Memory::AllocateArray(&cells, cap, capacity);
	capacity = BitCeiling(capacity) >> 1;
	capMinusOne = capacity - 1;
	defVal = def;
}

template<class Key, class Value> inline Hashmap<Key, Value>::Hashmap(const Hashmap& other) : size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	Memory::AllocateArray(&cells, capacity);
	Memory::Copy(cells, other.cells, sizeof(Cell) * capacity);
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
	Memory::Copy(cells, other.cells, sizeof(Cell) * capacity);

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

template<class Key, class Value> inline Hashmap<Key, Value>::~Hashmap()
{
	Destroy();
}

template<class Key, class Value> inline void Hashmap<Key, Value>::Destroy()
{
	if (cells)
	{
		if constexpr (IsStringType<Key>)
		{
			Cell* cell = cells;
			for (U64 i = 0; i < capacity; ++i)
			{
				if (cell->filled) { cell->key.Destroy(); }
				++cell;
			}
		}

		Memory::FreeArray(&cells);
		size = 0;
		capacity = 0;
		capMinusOne = 0;
	}

	if constexpr (IsDestroyable<Value>)
	{
		defVal.Destroy();
	}
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Insert(const Key& key, const Value& value)
{
	if (size == capacity) { return false; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = value;
	cell->key = key;

	return true;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Insert(const Key& key, Value&& value) noexcept
{
	if (size == capacity) { return false; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = Move(value);
	cell->key = key;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(const Key& key)
{
	if (size == 0) { return false; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;
		if constexpr (IsStringType<Key>) { cell->key.Destroy(); }
		Memory::Zero(cell, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(const Key& key, Value& value) noexcept
{
	if (size == 0) { return false; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;

		value = Move(cell->value);

		if constexpr (IsStringType<Key>) { cell->key.Destroy(); }
		Memory::Zero(cell, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline Value& Hashmap<Key, Value>::Get(const Key& key)
{
	if (size == 0) { return defVal; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return cell->value; }
	else { return defVal; }
}

template<class Key, class Value> inline Value& Hashmap<Key, Value>::Request(const Key& key)
{
	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	size += !cell->filled;

	cell->filled = true;
	cell->key = key;
	return cell->value;
}

template<class Key, class Value> inline Value& Hashmap<Key, Value>::Request(const Key& key, HashHandle& hnd)
{
	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	HashHandle handle = hash & capMinusOne;
	Cell* cell = cells + handle;
	while (cell->key != key && cell->filled) { ++i; cell = cells + (handle = ((hash + i * i) & capMinusOne)); }

	size += !cell->filled;

	hnd = handle;
	cell->filled = true;
	cell->key = key;
	return cell->value;
}

template<class Key, class Value> inline HashHandle Hashmap<Key, Value>::GetHandle(const Key& key)
{
	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	HashHandle handle = hash & capMinusOne;
	Cell* cell = cells + handle;
	while (cell->key != key) { ++i; cell = cells + (handle = ((hash + i * i) & capMinusOne)); }

	if (cell->filled) { return handle; }
	else { return U64_MAX; }
}

template<class Key, class Value> inline Value& Hashmap<Key, Value>::Obtain(HashHandle handle)
{
	return cells[handle].value;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(HashHandle handle)
{
	Cell& cell = cells[handle];

	if (cell.filled)
	{
		--size;

		if constexpr (IsStringType<Key>) { cell.key.Destroy(); }
		Memory::Zero(&cell, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline bool Hashmap<Key, Value>::Remove(HashHandle handle, Value& value) noexcept
{
	Cell& cell = cells[handle];

	if (cell.filled)
	{
		--size;

		value = Move(cell.value);

		if constexpr (IsStringType<Key>) { cell.key.Destroy(); }
		Memory::Zero(&cell, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value> inline Value& Hashmap<Key, Value>::operator[](const Key& key)
{
	if (size == 0) { return defVal; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return cell->value; }
	else { return defVal; }
}

template<class Key, class Value> inline const Value& Hashmap<Key, Value>::operator[](const Key& key) const
{
	if (size == 0) { return defVal; }

	U64 hash;
	if constexpr (IsStringType<Key>) { hash = key.Hash(); }
	else { hash = Hash::Calculate(key); }

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return cell->value; }
	else { return defVal; }
}

template<class Key, class Value> inline void Hashmap<Key, Value>::Reserve(U64 cap)
{
	if (cap < capacity) { return; }

	Memory::Reallocate(&cells, cap, capacity);
	capacity = BitCeiling(capacity) >> 1;
	capMinusOne = capacity - 1;

	Empty();
}

template<class Key, class Value> inline void Hashmap<Key, Value>::operator()(U64 capacity) { Reserve(capacity); }

template<class Key, class Value> inline void Hashmap<Key, Value>::Empty()
{
	if constexpr (IsStringType<Key>) { for (Cell& cell : *this) { cell.key.Destroy(); } }
	//TODO: Free memory if value is allocated

	Memory::Zero(cells, sizeof(Cell) * capacity);
	size = 0;
}

template<class Key, class Value> inline const U64& Hashmap<Key, Value>::Size() const { return size; }
template<class Key, class Value> inline const U64& Hashmap<Key, Value>::Capacity() const { return size; }

/*------ITERATOR------*/

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator::Iterator(Cell* cell) : cell{ cell } {}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator::Iterator(const Iterator& other) : cell{ cell } {}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator::Iterator(Iterator&& other) : cell{ cell } {}

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::Valid() const { return cell->filled; }

template<class Key, class Value>
inline Value& Hashmap<Key, Value>::Iterator::operator* () { return cell->value; }

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::Iterator::operator-> () { return &cell->value; }

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator Hashmap<Key, Value>::Iterator::operator++()
{
	Cell* temp = cell;
	++cell;

	return { temp };
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator& Hashmap<Key, Value>::Iterator::operator++(int)
{
	++cell;

	return *this;
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator Hashmap<Key, Value>::Iterator::operator--()
{
	Cell* temp = cell;
	--cell;

	return { temp };
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator& Hashmap<Key, Value>::Iterator::operator--(int)
{
	--cell;

	return *this;
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Iterator::operator bool() const { return cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator== (const Iterator& other) const { return cell == other.cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator!= (const Iterator& other) const { return cell != other.cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator< (const Iterator& other) const { return cell < other.cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator> (const Iterator& other) const { return cell > other.cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator<= (const Iterator& other) const { return cell <= other.cell; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Iterator::operator>= (const Iterator& other) const { return cell >= other.cell; }
