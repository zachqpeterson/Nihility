#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Platform/Memory.hpp"
#include "Math/Hash.hpp"

template<class Key, class Value>
struct Hashmap
{
	struct Cell
	{
		bool filled;
		Key key;
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

	Hashmap();
	Hashmap(U64 capacity);
	Hashmap(Hashmap&& other) noexcept;
	Hashmap(const Hashmap&);

	Hashmap& operator=(const Hashmap&);
	Hashmap& operator=(Hashmap&& other) noexcept;

	~Hashmap();
	void Destroy();

	bool Insert(const Key& key, const Value& value);
	bool Insert(const Key& key, Value&& value) noexcept;
	Value* GetInsert(const Key& key, const Value& value);
	Value* GetInsert(const Key& key, Value&& value) noexcept;
	bool Remove(const Key& key);

	Value* Get(const Key& key) const;
	Value* Request(const Key& key);
	Value* RequestWithHash(const Key& key, U64 hash);
	Value* Request(const Key& key, U64& handle);
	U64 GetHandle(const Key& key) const;
	Value* Obtain(U64 handle) const;
	bool Remove(U64 handle);

	Value* operator[](const Key& key);
	const Value* operator[](const Key& key) const;

	void Reserve(U64 capacity);
	void operator()(U64 capacity);
	void Clear();

	U64 Size() const;
	U64 Capacity() const;
	bool Empty() const;

	Iterator begin() { return { cells }; }
	const Iterator begin() const { return { cells }; }
	Iterator end() { return { cells + capacity }; }
	const Iterator end() const { return { cells + capacity }; }

private:
	U64 size = 0;
	U64 capacity = 0;
	U64 capMinusOne = 0;
	Cell* cells = nullptr;
};

template<class Key, class Value>
inline Hashmap<Key, Value>::Hashmap() {}

template<class Key, class Value>
inline Hashmap<Key, Value>::Hashmap(U64 cap)
{
	capacity = BitCeiling(cap);
	Memory::Allocate(&cells, capacity);
	capMinusOne = capacity - 1;
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Hashmap(const Hashmap& other) : size(other.size), capacity(other.capacity), capMinusOne(other.capMinusOne)
{
	Memory::Allocate(&cells, capacity);
	CopyData(cells, other.cells, capacity);
}

template<class Key, class Value>
inline Hashmap<Key, Value>::Hashmap(Hashmap&& other) noexcept :
	cells(other.cells), size(other.size), capacity(other.capacity), capMinusOne(other.capMinusOne)
{
	other.cells = nullptr;
	other.size = 0;
	other.capacity = 0;
	other.capMinusOne = 0;
}

template<class Key, class Value>
inline Hashmap<Key, Value>& Hashmap<Key, Value>::operator=(const Hashmap& other)
{
	size = other.size;
	capacity = other.capacity;
	capMinusOne = other.capMinusOne;

	Memory::Allocate(&cells, capacity);
	CopyData(cells, other.cells, capacity);

	return *this;
}

template<class Key, class Value>
inline Hashmap<Key, Value>& Hashmap<Key, Value>::operator=(Hashmap&& other) noexcept
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

template<class Key, class Value>
inline Hashmap<Key, Value>::~Hashmap()
{
	Destroy();
}

template<class Key, class Value>
inline void Hashmap<Key, Value>::Destroy()
{
	if (cells)
	{
		if constexpr (IsDestroyable<Key> || IsDestroyable<Value>)
		{
			Cell* cell = cells;
			for (U64 i = 0; i < capacity; ++i, ++cell)
			{
				if (cell->filled)
				{
					//TODO: Key or Value could be allocated
					if constexpr (IsDestroyable<Key>)
					{
						if constexpr (IsPointer<Key>) { cell->key->Destroy(); }
						else { cell->key.Destroy(); }
					}
					if constexpr (IsDestroyable<Value>)
					{
						if constexpr (IsPointer<Value>) { cell->value->Destroy(); }
						else { cell->value.Destroy(); }
					}
				}
			}
		}

		Memory::Free(&cells);
		size = 0;
		capacity = 0;
		capMinusOne = 0;
	}
}

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Insert(const Key& key, const Value& value)
{
	if (size == capacity) { return false; }

	U64 hash = Hash::Any(key);

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);

	while (cell->filled) { if (cell->key == key) { return false; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = value;
	cell->key = key;

	return true;
}

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Insert(const Key& key, Value&& value) noexcept
{
	if (size == capacity) { return false; }

	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);

	while (cell->filled) { if (cell->key == key) { return false; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = Move(value);
	cell->key = key;

	return true;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::GetInsert(const Key& key, const Value& value)
{
	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return &cell->value; }

	++size;

	cell->filled = true;
	cell->key = key;
	cell->value = value;
	return &cell->value;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::GetInsert(const Key& key, Value&& value) noexcept
{
	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return &cell->value; }

	++size;

	cell->filled = true;
	cell->key = key;
	cell->value = Move(value);
	return &cell->value;
}

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Remove(const Key& key)
{
	if (size == 0) { return false; }

	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;
		if constexpr (IsDestroyable<Key>)
		{
			if constexpr (IsPointer<Value>) { cell->key->Destroy(); }
			else { cell->key.Destroy(); }
		}
		if constexpr (IsDestroyable<Value>)
		{
			if constexpr (IsPointer<Value>) { cell->value->Destroy(); }
			else { cell->value.Destroy(); }
		}
		memset(cell, 0, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::Get(const Key& key) const
{
	if (size == 0) { return nullptr; }

	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return &cell->value; }
	return nullptr;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::Request(const Key& key)
{
	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);

	while (cell->filled) { if (cell->key == key) { return &cell->value; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;

	cell->filled = true;
	cell->key = key;
	return &cell->value;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::RequestWithHash(const Key& key, U64 hash)
{
	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);

	while (cell->filled) { if (cell->key == key) { return &cell->value; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;

	cell->filled = true;
	cell->key = key;
	return &cell->value;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::Request(const Key& key, U64& handle)
{
	U64 hash = Hash::Any(key);

	U64 i = 0;
	handle = hash & capMinusOne;
	Cell* cell = cells + handle;

	while (cell->filled) { if (cell->key == key) { return &cell->value; } ++i; cell = cells + (handle = ((hash + i * i) & capMinusOne)); }

	size += !cell->filled;

	cell->filled = true;
	cell->key = key;
	return &cell->value;
}

template<class Key, class Value>
inline U64 Hashmap<Key, Value>::GetHandle(const Key& key) const
{
	U64 hash = Hash::Any(key);

	U64 i = 0;
	U64 handle = hash & capMinusOne;
	Cell* cell = cells + handle;
	while (cell->filled && cell->key != key) { ++i; cell = cells + (handle = ((hash + i * i) & capMinusOne)); }

	if (cell->filled) { return handle; }
	else { return U64_MAX; }
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::Obtain(U64 handle) const
{
	return &cells[handle].value;
}

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Remove(U64 handle)
{
	Cell& cell = cells[handle];

	if (cell.filled)
	{
		--size;

		if constexpr (IsDestroyable<Key>)
		{
			if constexpr (IsPointer<Value>) { cell.key->Destroy(); }
			else { cell.key.Destroy(); }
		}
		if constexpr (IsDestroyable<Value>)
		{
			if constexpr (IsPointer<Value>) { cell.value->Destroy(); }
			else { cell.value.Destroy(); }
		}
		memset(&cell, 0, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Key, class Value>
inline Value* Hashmap<Key, Value>::operator[](const Key& key)
{
	if (size == 0) { return nullptr; }

	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return &cell->value; }
	else { return nullptr; }
}

template<class Key, class Value>
inline const Value* Hashmap<Key, Value>::operator[](const Key& key) const
{
	if (size == 0) { return nullptr; }

	U64 hash = Hash::Any(key);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->key != key && cell->filled) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled) { return &cell->value; }
	else { return nullptr; }
}

template<class Key, class Value>
inline void Hashmap<Key, Value>::Reserve(U64 cap)
{
	if (cap <= capacity) { return; }

	capacity = BitFloor(cap);
	Memory::Reallocate(&cells, cap);
	capMinusOne = capacity - 1;

	Clear();
}

template<class Key, class Value>
inline void Hashmap<Key, Value>::operator()(U64 capacity) { Reserve(capacity); }

template<class Key, class Value>
inline void Hashmap<Key, Value>::Clear()
{
	if constexpr (IsDestroyable<Key> || IsDestroyable<Value>)
	{
		Cell* cell = cells;
		for (U64 i = 0; i < capacity; ++i, ++cell)
		{
			if (cell->filled)
			{
				if constexpr (IsDestroyable<Key>)
				{
					if constexpr (IsPointer<Key>) { cell->key->Destroy(); }
					else { cell->key.Destroy(); }
				}
				if constexpr (IsDestroyable<Value>)
				{
					if constexpr (IsPointer<Value>) { cell->value->Destroy(); }
					else { cell->value.Destroy(); }
				}
			}
		}
	}

	memset(cells, 0, sizeof(Cell) * capacity);
	size = 0;
}

template<class Key, class Value>
inline U64 Hashmap<Key, Value>::Size() const { return size; }

template<class Key, class Value>
inline U64 Hashmap<Key, Value>::Capacity() const { return size; }

template<class Key, class Value>
inline bool Hashmap<Key, Value>::Empty() const { return size == 0; }

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