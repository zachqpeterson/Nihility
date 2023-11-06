#pragma once

#include "ContainerDefines.hpp"
#include "String.hpp"
#include "Memory\Memory.hpp"
#include "Math\Hash.hpp"

template<class Value>
struct Hashset
{
	struct Cell
	{
		bool filled;
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
	Hashset();
	Hashset(U64 capacity);
	Hashset(Hashset&& other) noexcept;
	Hashset& operator=(Hashset&& other) noexcept;

	~Hashset();
	void Destroy();

	bool Insert(const Value& value);
	bool Insert(Value&& value) noexcept;
	bool Remove(const Value& value);

	bool Get(const Value& value) const;
	bool operator[](const Value& value) const;

	void Reserve(U64 capacity);
	void operator()(U64 capacity);
	void Clear();

	U64 Size() const;
	U64 Capacity() const;

	Iterator begin() { return { cells }; }
	const Iterator begin() const { return { cells }; }
	Iterator end() { return { cells + capacity }; }
	const Iterator end() const { return { cells + capacity }; }

private:
	U64 Hash(const Value& value);

	U64 size{ 0 };
	U64 capacity{ 0 };
	U64 capMinusOne{ 0 };
	Cell* cells{ nullptr };

	Hashset(const Hashset&) = delete;
	Hashset& operator=(const Hashset&) = delete;
};

template<class Value> 
inline Hashset<Value>::Hashset() {}

template<class Value> 
inline Hashset<Value>::Hashset(U64 cap)
{
	Memory::AllocateArray(&cells, cap, capacity);
	capacity = BitFloor(capacity);
	capMinusOne = capacity - 1;
}

template<class Value> 
inline Hashset<Value>::Hashset(Hashset&& other) noexcept :
	cells{ other.cells }, size{ other.size }, capacity{ other.capacity }, capMinusOne{ other.capMinusOne }
{
	other.cells = nullptr;
	other.size = 0;
	other.capacity = 0;
	other.capMinusOne = 0;
}

template<class Value> 
inline Hashset<Value>& Hashset<Value>::operator=(Hashset&& other) noexcept
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

template<class Value> 
inline Hashset<Value>::~Hashset()
{
	Destroy();
}

template<class Value> 
inline void Hashset<Value>::Destroy()
{
	if (cells)
	{
		if constexpr (IsDestroyable<Value>)
		{
			Cell* cell = cells;
			for (U64 i = 0; i < capacity; ++i, ++cell)
			{
				if (cell->filled) { cell->value.Destroy(); }
			}
		}

		Memory::Free(&cells);
		size = 0;
		capacity = 0;
		capMinusOne = 0;
	}
}

template<class Value> 
inline bool Hashset<Value>::Insert(const Value& value)
{
	if (size == capacity) { return false; }

	U64 hash = Hash(value);

	U32 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { if (cell->value == value) { return false; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = value;

	return true;
}

template<class Value> 
inline bool Hashset<Value>::Insert(Value&& value) noexcept
{
	if (size == capacity) { return false; }

	U64 hash = Hash(value);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled) { if (cell->value == value) { return false; } ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	++size;
	cell->filled = true;
	cell->value = Move(value);

	return true;
}

template<class Value> 
inline bool Hashset<Value>::Remove(const Value& value)
{
	if (size == 0) { return false; }

	U64 hash = Hash(value);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->value != value) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	if (cell->filled)
	{
		--size;
		if constexpr (IsDestroyable<Value>)
		{
			if constexpr (IsPointer<Value>) { cell->value->Destroy(); }
			else { cell->value.Destroy(); }
		}
		Memory::Zero(cell, sizeof(Cell));

		return true;
	}

	return false;
}

template<class Value> 
inline bool Hashset<Value>::Get(const Value& value) const
{
	if (size == 0) { return false; }

	U64 hash = Hash(value);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	return cell->filled;
}

template<class Value> 
inline bool Hashset<Value>::operator[](const Value& value) const
{
	if (size == 0) { return false; }

	U64 hash = Hash(value);

	U64 i = 0;
	Cell* cell = cells + (hash & capMinusOne);
	while (cell->filled && cell->key != key) { ++i; cell = cells + ((hash + i * i) & capMinusOne); }

	return cell->filled;
}

template<class Value> 
inline void Hashset<Value>::Reserve(U64 cap)
{
	if (cap < capacity) { return; }

	Memory::Reallocate(&cells, cap, capacity);
	capacity = BitFloor(capacity);
	capMinusOne = capacity - 1;

	Clear();
}

template<class Value> 
inline void Hashset<Value>::operator()(U64 capacity) { Reserve(capacity); }

template<class Value> 
inline void Hashset<Value>::Clear()
{
	if constexpr (IsDestroyable<Value>)
	{
		Cell* cell = cells;
		for (U64 i = 0; i < capacity; ++i, ++cell)
		{
			if (cell->filled) { cell->value.Destroy(); }
		}
	}

	Memory::Zero(cells, sizeof(Cell) * capacity);
	size = 0;
}

template<class Value> 
inline U64 Hashset<Value>::Size() const { return size; }
template<class Value> 
inline U64 Hashset<Value>::Capacity() const { return size; }

/*------ITERATOR------*/

template<class Value>
inline Hashset<Value>::Iterator::Iterator(Cell* cell) : cell{ cell } {}

template<class Value>
inline Hashset<Value>::Iterator::Iterator(const Iterator& other) : cell{ cell } {}

template<class Value>
inline Hashset<Value>::Iterator::Iterator(Iterator&& other) : cell{ cell } {}

template<class Value>
inline bool Hashset<Value>::Iterator::Valid() const { return cell->filled; }

template<class Value>
inline Value& Hashset<Value>::Iterator::operator* () { return cell->value; }

template<class Value>
inline Value* Hashset<Value>::Iterator::operator-> () { return &cell->value; }

template<class Value>
inline Hashset<Value>::Iterator Hashset<Value>::Iterator::operator++()
{
	Cell* temp = cell;
	++cell;

	return { temp };
}

template<class Value>
inline Hashset<Value>::Iterator& Hashset<Value>::Iterator::operator++(int)
{
	++cell;

	return *this;
}

template<class Value>
inline Hashset<Value>::Iterator Hashset<Value>::Iterator::operator--()
{
	Cell* temp = cell;
	--cell;

	return { temp };
}

template<class Value>
inline Hashset<Value>::Iterator& Hashset<Value>::Iterator::operator--(int)
{
	--cell;

	return *this;
}

template<class Value>
inline Hashset<Value>::Iterator::operator bool() const { return cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator== (const Iterator& other) const { return cell == other.cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator!= (const Iterator& other) const { return cell != other.cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator< (const Iterator& other) const { return cell < other.cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator> (const Iterator& other) const { return cell > other.cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator<= (const Iterator& other) const { return cell <= other.cell; }

template<class Value>
inline bool Hashset<Value>::Iterator::operator>= (const Iterator& other) const { return cell >= other.cell; }

template<class Value>
inline U64 Hashset<Value>::Hash(const Value& value)
{
	if constexpr (IsStringType<Value>) { return value.Hash(); }
	else if constexpr (IsPointer<Value>) { return Hash::Calculate(static_cast<U64>(value)); }
	else { return Hash::Calculate(value); }
}