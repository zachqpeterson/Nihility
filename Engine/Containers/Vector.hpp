#pragma once

#include "Defines.hpp"

#include "Platform/Memory.hpp"

#include <initializer_list>

template<class Type>
struct NH_API Vector
{
	/// <summary>
	/// Creates a new Vector instance, size and capacity will be zero, array will be nullptr
	/// </summary>
	Vector();

	/// <summary>
	/// Creates a new Vector instance, size will be zero, creates an array of size greater than or equal to sizeof(T) * capacity
	/// </summary>
	/// <param name="capacity:">The capacity the array will be at</param>
	Vector(U64 capacity);

	/// <summary>
	/// Creates a new Vector instance, capacity will be greater than or equal to size, creates an array of size sizeof(T) * capacity and fills it with value
	/// </summary>
	/// <param name="size:">The size the array will be</param>
	/// <param name="value:">The value that the array will be filled with</param>
	Vector(U64 size, const Type& value);

	/// <summary>
	/// Create a new Vector instance using an initializer list, size will equal the list size, capacity with be greater than or equal to size, creates an array of size sizeof(T) * capacity and fills it with the values in list
	/// </summary>
	/// <param name="list:">The initializer list</param>
	Vector(std::initializer_list<Type> list);

	/// <summary>
	/// Creates a new Vector instance, capacity and size will be other's, creates an array of the same size and copies other's data into it
	/// </summary>
	/// <param name="other:">Vector to copy</param>
	Vector(const Vector& other);

	/// <summary>
	/// Creates a new Vector instance, capacity and size will be other's, sets the array to other's 
	/// <para/>WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	Vector(Vector&& other) noexcept;

	/// <summary>
	/// Copies other's data into this, capacity and size will be other's, creates an array of the same size and copies other's data into it
	/// <para/>WARNING: any previous data will be lost
	/// </summary>
	/// <param name="other:">The Vector to copy</param>
	/// <returns>Reference to this</returns>
	Vector& operator=(const Vector& other);

	/// <summary>
	/// Moves other's data into this, capacity and size will be other's, sets the array to other's 
	/// <para/>WARNING: any previous data will be lost
	/// <para/>WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	/// <returns>Reference to this</returns>
	Vector& operator=(Vector&& other) noexcept;



	~Vector();

	/// <summary>
	/// Destroys data inside this, capacity and size will be zero, array will be nullptr
	/// </summary>
	void Destroy();



	/// <summary>
	/// Increases size by one and puts value onto the back of array, reallocates array if it's too small
	/// </summary>
	/// <param name="value:">The value to put into array</param>
	Type& Push(const Type& value);

	/// <summary>
	/// Increases size by one and moves value onto the back of array, reallocates array if it's too small
	/// </summary>
	/// <param name="value:">The value to move into array</param>
	Type& Push(Type&& value) noexcept;

	/// <summary>
	/// Increases size by one without cunstructing an object at the new index, reallocates array if it's too small
	/// </summary>
	/// <returns>Pointer to the new spot in the array</returns>
	Type* PushEmpty();

	/// <summary>
	/// Increases size by one and constructs value into the back of array using the parameters, reallocates array if it's too small
	/// </summary>
	/// <param name="parameters:">The parameters to construct with</param>
	/// <returns>A refernce to the value</returns>
	template <class... Parameters>
	Type& Emplace(Parameters&&... parameters) noexcept;

	/// <summary>
	/// constructs value into the index of array using the parameters.
	/// <para/>WARNING: any previous data at index will be lost
	/// </summary>
	/// <param name="parameters:">The parameters to construct with</param>
	/// <returns>A refernce to the value</returns>
	template <Unsigned I, class... Parameters>
	Type& EmplaceAt(I index, Parameters&&... parameters) noexcept;

	/// <summary>
	/// Decreases the size by one
	/// </summary>
	void Pop();

	/// <summary>
	/// Decreases the size by one and moves what was in the back of array to value
	/// </summary>
	/// <param name="value:">The value to move to</param>
	void Pop(Type& value);

	/// <summary>
	/// Inserts value into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to put value</param>
	/// <param name="value:">The value to copy</param>
	/// <returns>A refernce to the value</returns>
	template<Unsigned Index> Type& Insert(Index index, const Type& value);

	/// <summary>
	/// Inserts value into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to put value</param>
	/// <param name="value:">The value to move</param>
	/// <returns>A refernce to the value</returns>
	template<Unsigned Index> Type& Insert(Index index, Type&& value) noexcept;

	/// <summary>
	/// Copies other and inserts it into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to copy other into</param>
	/// <param name="other:">The Vector to copy</param>
	template<Unsigned Index> void Insert(Index index, const Vector& other);

	/// <summary>
	/// Moves other and inserts it into index, moves values at and past index over, reallocates array if it's too small
	/// <para/>WARNING: other will be destroyed
	/// </summary>
	/// <param name="index:">The index to move other into</param>
	/// <param name="other:">The Vector to move</param>
	template<Unsigned Index> void Insert(Index index, Vector&& other) noexcept;

	/// <summary>
	/// Moves values past index to index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	void Remove(U64 index);

	/// <summary>
	/// Copies value at index into value, moves values past index to index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	/// <param name="value:">The value to copy to</param>
	void Remove(U64 index, Type& value);

	/// <summary>
	/// Removes the value at index by swaping it with the last index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	I32 RemoveSwap(U64 index);

	/// <summary>
	/// Copies value at index into value, removes the value at index by swaping it with the last index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	/// <param name="value:">The value to copy to</param>
	I32 RemoveSwap(U64 index, Type& value);

	/// <summary>
	/// Moves values at and past index1 to index0
	/// </summary>
	/// <param name="index0:">The beginning of the erasure, inclusive</param>
	/// <param name="index1:">The end of the erasure, exclusive</param>
	void Erase(U64 index0, U64 index1);

	/// <summary>
	/// Copies values to be erased into other, moves values at and past index1 to index0
	/// <para/>WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="index0:">The beginning of the erasure, inclusive</param>
	/// <param name="index1:">The end of the erasure, exclusive</param>
	/// <param name="other:">The Vector to copy to</param>
	void Erase(U64 index0, U64 index1, Vector& other);



	/// <summary>
	/// Splits the array at index, copies data at and past index into other
	/// <para/>WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="index:">The index to split on, inclusive</param>
	/// <param name="other:">The Vector to copy to</param>
	void Split(U64 index, Vector& other);

	/// <summary>
	/// Copies data in other to the end of the array, reallocates the array if it's too small
	/// </summary>
	/// <param name="other:">The Vector to copy from</param>
	void Merge(const Vector& other);

	/// <summary>
	/// Moves data in other to the end of the array, reallocates the array if it's too small
	/// <para/>WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	void Merge(Vector&& other) noexcept;

	/// <summary>
	/// Copies data in other to the end of the array, reallocates the array if it's too small
	/// </summary>
	/// <param name="other:">The Vector to copy from</param>
	/// <returns>Reference to this</returns>
	Vector& operator+=(const Vector& other);

	/// <summary>
	/// Moves data in other to the end of the array, reallocates the array if it's too small
	/// <para/>WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	/// <returns>Reference to this</returns>
	Vector& operator+=(Vector&& other) noexcept;



	/// <summary>
	/// Searches array, finds all values that satisfy predicate, fill other with those values
	/// <para/>WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <param name="other:">A Vector to fill with values</param>
	template<FunctionPtr Predicate> void SearchFor(Predicate predicate, Vector& other);

	/// <summary>
	/// Searches array, finds indices of all values that satisfy predicate, fill other with those indices
	/// <para/>WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <param name="other:">A Vector to fill with indices</param>
	template<FunctionPtr Predicate> void SearchForIndices(Predicate predicate, Vector<U64>& other);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <returns>The count of values that satisfy predicate</returns>
	template<FunctionPtr Predicate> U64 SearchCount(Predicate predicate);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate, removes them from array
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <returns>The count of values that satisfy predicate</returns>
	template<FunctionPtr Predicate> U64 RemoveAll(Predicate predicate);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate, removes them from array and puts them into other
	/// <para/>WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <param name="other:">A Vector to fill with values</param>
	template<FunctionPtr Predicate> void RemoveAll(Predicate predicate, Vector& other);

	/// <summary>
	/// Finds the first value that satisfies the predicate, return true if one exists
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const Type&amp; value)</param>
	/// <param name="value:">A reference to return the found value</param>
	/// <returns>true if a value exists, false otherwise</returns>
	template<FunctionPtr Predicate> Type* Find(Predicate predicate) const;



	/// <summary>
	/// Inserts a value based on a predicate
	/// </summary>
	/// <param name="predicate:">A function to compare values: bool pred(const Type&amp; a, const Type&amp; b)</param>
	/// <param name="value:">The value to insert</param>
	/// <returns>The index the value was inserted at</returns>
	template<FunctionPtr Predicate> U64 SortedInsert(Predicate predicate, const Type& value);

	/// <summary>
	/// Inserts a value based on a predicate
	/// </summary>
	/// <param name="predicate:">A function to compare values: bool pred(const Type&amp; a, const Type&amp; b)</param>
	/// <param name="value:">The value to insert</param>
	/// <returns>The index the value was inserted at</returns>
	template<FunctionPtr Predicate> U64 SortedInsert(Predicate predicate, Type&& value) noexcept;



	/// <summary>
	/// Reallocates the array to be sizeof(T) * capacity
	/// </summary>
	/// <param name="capacity:">The capacity the array will be at</param>
	void Reserve(U64 capacity);

	/// <summary>
	/// Sets size, reallocates the array if it's too small
	/// </summary>
	/// <param name="size:">The size to set to</param>
	void Resize(U64 size);

	/// <summary>
	/// Sets size, fills the array with value, reallocates the array if it's too small
	/// </summary>
	/// <param name="size:">The size to set to</param>
	/// <param name="value:">The value to fill the array with</param>
	void Resize(U64 size, const Type& value);

	/// <summary>
	/// Sets size to zero
	/// </summary>
	void Clear();



	/// <summary>
	/// Searches array for value
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>true if value is contained within array, false otherwise</returns>
	bool Contains(const Type& value) const;

	/// <summary>
	/// Counts the reoccurrences of value in array
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>The number of reoccurrences of value</returns>
	U64 Count(const Type& value) const;

	/// <summary>
	/// Finds the first index of value
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>The index of value, if it doesn't find value, U64_MAX</returns>
	U64 Find(const Type& value) const;

	/// <summary>
	/// Gets the index of value
	/// </summary>
	/// <param name="value:">The value to get the index of</param>
	/// <returns>The index of value, if the value isn't in the array, U64_MAX</returns>
	U64 Index(const Type* value) const;



	/// <returns>The current amount of elements</returns>
	U64 Size() const { return size; }

	/// <returns>The current maximum allowed elements</returns>
	U64 Capacity() const { return capacity; }

	/// <returns>Whether or not this is empty</returns>
	bool Empty() const { return size == 0; }

	/// <returns>Whether or not this is full</returns>
	bool Full() const { return size == capacity; }

	/// <summary></summary>
	/// <returns>array (const)</returns>
	const Type* Data() const { return array; }

	/// <summary></summary>
	/// <returns>array</returns>
	Type* Data() { return array; }



	/// <summary>Retrieves the value at an index</summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index (const)</returns>
	const Type& Get(U64 i) const { return array[i]; }

	/// <summary>Retrieves the value at an index</summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index</returns>
	Type& Get(U64 i) { return array[i]; }

	/// <summary>Retrieves the value at an index</summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index (const)</returns>
	const Type& operator[](U64 i) const { if (i >= size) { BreakPoint; } return array[i]; }

	/// <summary>Retrieves the value at an index</summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index</returns>
	Type& operator[](U64 i) { if (i >= size) { BreakPoint; } return array[i]; }

	/// <summary>
	/// Gets the value at the front of array (index 0)
	/// </summary>
	/// <returns>The value at the front of array</returns>
	Type& Front() { return *array; }

	/// <summary>
	/// Gets the value at the front of array (index 0)
	/// </summary>
	/// <returns>The value at the front of array (const)</returns>
	const Type& Front() const { return *array; }

	/// <summary>
	/// Gets the value at the back of array (index size - 1)
	/// </summary>
	/// <returns>The value at the back of array</returns>
	Type& Back() { return array[size - 1]; }

	/// <summary>
	/// Gets the value at the back of array (index size - 1)
	/// </summary>
	/// <returns>The value at the back of array (const)</returns>
	const Type& Back() const { return array[size - 1]; }



	/// <summary>
	/// Compares the values stored in both vectors
	/// </summary>
	/// <param name="other: ">The other vector to compare against</param>
	/// <returns>True if the two vectors have the same values</returns>
	bool operator==(const Vector& other) const;

	/// <summary>
	/// Compares the values stored in both vectors
	/// </summary>
	/// <param name="other: ">The other vector to compare against</param>
	/// <returns>True if the two vectors have different values</returns>
	bool operator!=(const Vector& other) const;



	/// <summary></summary>
	/// <returns>The beginning of array as an iterator</returns>
	Type* begin() { return array; }

	/// <summary></summary>
	/// <returns>The end of array as an iterator</returns>
	Type* end() { return array + size; }

	/// <summary></summary>
	/// <returns>The beginning of array as an iterator (const)</returns>
	const Type* begin() const { return array; }

	/// <summary></summary>
	/// <returns>The end of array as an iterator (const)</returns>
	const Type* end() const { return array + size; }

private:

	/// <summary>
	/// The count of values inside array
	/// </summary>
	U64 size = 0;

	/// <summary>
	/// The actual size of array
	/// </summary>
	U64 capacity = 0;

	/// <summary>
	/// A dynamically allocated array to store data
	/// </summary>
	Type* array = nullptr;
};

template<class Type> inline Vector<Type>::Vector() {}

template<class Type> inline Vector<Type>::Vector(U64 cap) { capacity = Memory::Allocate(&array, cap); }

template<class Type> inline Vector<Type>::Vector(U64 size, const Type& value) : size(size), capacity(size)
{
	capacity = Memory::Allocate(&array, capacity);
	for (Type* t = array, *end = array + size; t != end; ++t) { *t = value; }
}

template<class Type> inline Vector<Type>::Vector(std::initializer_list<Type> list) : size(list.size()), capacity(size)
{
	capacity = Memory::Allocate(&array, capacity);
	CopyData(array, list.begin(), size);
}

template<class Type> inline Vector<Type>::Vector(const Vector<Type>& other) : size(other.size), capacity(other.size)
{
	capacity = Memory::Allocate(&array, capacity);
	CopyData(array, other.array, size);
}

template<class Type> inline Vector<Type>::Vector(Vector<Type>&& other) noexcept : size(other.size), capacity(other.capacity), array(other.array)
{
	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;
}

template<class Type> inline Vector<Type>& Vector<Type>::operator=(const Vector<Type>& other)
{
	size = other.size;
	if (capacity < other.size) { capacity = Memory::Reallocate(&array, size); }

	CopyData(array, other.array, size);

	return *this;
}

template<class Type> inline Vector<Type>& Vector<Type>::operator=(Vector<Type>&& other) noexcept
{
	if (array) { Memory::Free(&array); }
	size = other.size;
	capacity = other.capacity;
	array = other.array;

	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;

	return *this;
}

template<class Type> inline Vector<Type>::~Vector() { Destroy(); }

template<class Type> inline void Vector<Type>::Destroy()
{
	if (array)
	{
		if constexpr (IsDestructible<Type>)
		{
			for (Type* it = array, *end = array + size; it != end; ++it) { it->~Type(); }
		}

		Memory::Free(&array);
	}

	size = 0;
	capacity = 0;
}

template<class Type> inline Type& Vector<Type>::Push(const Type& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	return Construct<Type>(array + size++, value);
}

template<class Type> inline Type& Vector<Type>::Push(Type&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	return Construct<Type>(array + size++, Move(value));
}

template<class Type> inline Type* Vector<Type>::PushEmpty()
{
	if (size == capacity) { Reserve(capacity + 1); }

	return array + size++;
}

template<class Type>
template <class... Parameters>
inline Type& Vector<Type>::Emplace(Parameters&&... parameters) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	return Construct<Type, Parameters...>(array + size++, Forward<Parameters>(parameters)...);
}

template<class Type>
template <Unsigned I, class... Parameters>
inline Type& Vector<Type>::EmplaceAt(I index, Parameters&&... parameters) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	return Construct<Type, Parameters...>(array + index, Forward<Parameters>(parameters)...);
}

template<class Type> inline void Vector<Type>::Pop()
{
	if (size)
	{
		if constexpr (IsDestructible<Type>) { (array + size - 1)->~Type(); }
		--size;
	}
}

template<class Type> inline void Vector<Type>::Pop(Type& value)
{
	if (size) { Construct<Type>(&value, Move(array[--size])); }
}

template<class Type>
template<Unsigned I>
inline Type& Vector<Type>::Insert(I index, const Type& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	MoveData(array + index + 1, array + index, (size - index));
	++size;
	return Construct<Type>(array + index, value);
}

template<class Type>
template<Unsigned I>
inline Type& Vector<Type>::Insert(I index, Type&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	MoveData(array + index + 1, array + index, (size - index));
	++size;
	return Construct<Type>(array + index, Move(value));
}

template<class Type>
template<Unsigned I>
inline void Vector<Type>::Insert(I index, const Vector<Type>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	MoveData(array + index + other.size, array + index, (size - index));
	CopyData(array + index, other.array, other.size);

	size += other.size;
}

template<class Type>
template<Unsigned I>
inline void Vector<Type>::Insert(I index, Vector<Type>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	MoveData(array + index + other.size, array + index, (size - index));
	MoveData(array + index, other.array, other.size);
	size += other.size;

	other.Destroy();
}

template<class Type> inline void Vector<Type>::Remove(U64 index)
{
	MoveData(array + index, array + index + 1, (size - index));

	--size;
}

template<class Type> inline void Vector<Type>::Remove(U64 index, Type& value)
{
	return Construct<Type>(&value, Move(array[index]));
	MoveData(array + index, array + index + 1, (size - index));

	--size;
}

template<class Type> inline I32 Vector<Type>::RemoveSwap(U64 index)
{
	if (index < size - 1)
	{
		Assign(array + index, Move(array[--size]));
		return (I32)size;
	}

	--size;
	return -1;
}

template<class Type> inline I32 Vector<Type>::RemoveSwap(U64 index, Type& value)
{
	Construct<Type>(&value, Move(array[index]));

	if (index < size - 1)
	{
		Construct<Type>(array + index, Move(array[--size]));
		return size;
	}

	--size;
	return -1;
}

template<class Type> inline void Vector<Type>::Erase(U64 index0, U64 index1)
{
	MoveData(array + index0, array + index1, (size - index1));

	size -= index1 - index0;
}

template<class Type> inline void Vector<Type>::Erase(U64 index0, U64 index1, Vector<Type>& other)
{
	other.Reserve(index1 - index0);
	other.size = other.capacity;

	MoveData(other.array, array + index0, (index1 - index0));
	MoveData(array + index0, array + index1, (size - index1));

	size -= index1 - index0;
}

template<class Type> inline void Vector<Type>::Split(U64 index, Vector<Type>& other)
{
	other.Reserve(size - index);
	other.size = other.capacity;

	MoveData(other.array, array + index, other.size);

	size -= index;
}

template<class Type> inline void Vector<Type>::Merge(const Vector<Type>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	CopyData(array + size, other.array, other.size);
	size += other.size;
}

template<class Type> inline void Vector<Type>::Merge(Vector<Type>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	MoveData(array + size, other.array, other.size);
	size += other.size;

	other.Destroy();
}

template<class Type> inline Vector<Type>& Vector<Type>::operator+=(const Vector<Type>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	CopyData(array + size, other.array, other.size);
	size += other.size;

	return *this;
}

template<class Type> inline Vector<Type>& Vector<Type>::operator+=(Vector<Type>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	MoveData(array + size, other.array, other.size);
	size += other.size;

	other.Destroy();

	return *this;
}

template<class Type>
template<FunctionPtr Predicate>
inline void Vector<Type>::SearchFor(Predicate predicate, Vector<Type>& other)
{
	other.Reserve(size);
	other.size = 0;

	for (Type* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(*t)) { other.Push(*t); }
	}
}

template<class Type>
template<FunctionPtr Predicate>
inline void Vector<Type>::SearchForIndices(Predicate predicate, Vector<U64>& other)
{
	other.Reserve(size);
	other.size = 0;

	U64 i = 0;
	for (Type* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(*t)) { other.Push(i); }
	}
}

template<class Type>
template<FunctionPtr Predicate>
inline U64 Vector<Type>::SearchCount(Predicate predicate)
{
	U64 i = 0;
	for (Type* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(*t)) { ++i; }
	}

	return i;
}

template<class Type>
template<FunctionPtr Predicate>
inline U64 Vector<Type>::RemoveAll(Predicate predicate)
{
	Type* last = array + size;

	U64 i = 0;
	for (Type* t = array, *end = array + size; t != end;)
	{
		if (predicate(*t))
		{
			++i;
			MoveData(t, t + 1, (last - t - 1));
			--size;
		}
		else { ++t; }
	}

	return i;
}

template<class Type>
template<FunctionPtr Predicate>
inline void Vector<Type>::RemoveAll(Predicate predicate, Vector<Type>& other)
{
	Type* last = array + size;

	other.Reserve(size);
	other.size = 0;

	for (Type* t = array, *end = array + size; t != end;)
	{
		if (predicate(*t))
		{
			other.Push(Move(*t));
			MoveData(t, array + size-- - 1, 1);
		}
		else { ++t; }
	}
}

template<class Type>
template<FunctionPtr Predicate>
inline Type* Vector<Type>::Find(Predicate predicate) const
{
	for (Type* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(t)) { return t; }
	}

	return nullptr;
}

template<class Type>
template<FunctionPtr Predicate>
U64 Vector<Type>::SortedInsert(Predicate predicate, const Type& value)
{
	U64 i = 0;
	for (Type* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(value, *t))
		{
			if (size == capacity) { Reserve(capacity + 1); }

			MoveData(array + i + 1, array + i, (size - i));
			Construct<Type>(array + i, value);
			++size;

			return i;
		}
	}

	U64 index = size;

	Push(value);

	return index;
}

template<class Type>
template<FunctionPtr Predicate>
U64 Vector<Type>::SortedInsert(Predicate predicate, Type&& value) noexcept
{
	U64 i = 0;
	for (Type* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(value, *t))
		{
			if (size == capacity) { Reserve(capacity + 1); }

			MoveData(array + i + 1, array + i, (size - i));
			Construct<Type>(array + i, Move(value));
			++size;

			return i;
		}
	}

	U64 index = size;

	Push(Move(value));

	return index;
}

template<class Type>
inline void Vector<Type>::Reserve(U64 cap)
{
	capacity = Memory::Reallocate(&array, cap);
}

template<class Type>
inline void Vector<Type>::Resize(U64 size)
{
	if (size > capacity) { Reserve(size); }
	this->size = size;
}

template<class Type>
inline void Vector<Type>::Resize(U64 size, const Type& value)
{
	if (size > capacity) { Reserve(size); }
	this->size = size;

	for (U64 i = 0; i < size; ++i) { Construct<Type>(array + i, value); }
}

template<class Type>
inline void Vector<Type>::Clear()
{
	if (array)
	{
		if constexpr (IsDestructible<Type>)
		{
			for (U64 i = 0; i < size; i++) { (array + i)->~Type(); }
		}
	}

	size = 0;
}

template<class Type>
inline bool Vector<Type>::Contains(const Type& value) const
{
	for (Type* t = array, *end = array + size; t != end; ++t)
	{
		if (*t == value) { return true; }
	}

	return false;
}

template<class Type>
inline U64 Vector<Type>::Count(const Type& value) const
{
	U64 count = 0;
	for (Type* t = array, *end = array + size; t != end; ++t)
	{
		if (*t == value) { ++count; }
	}

	return count;
}

template<class Type>
inline U64 Vector<Type>::Find(const Type& value) const
{
	U64 index = 0;
	for (Type* t = array; index < size; ++index, ++t)
	{
		if (*t == value) { return index; }
	}

	return U64_MAX;
}

template<class Type>
inline U64 Vector<Type>::Index(const Type* value) const
{
	if (value < array || value > array + capacity) { return U64_MAX; }

	return value - array;
}

template<class Type>
inline bool Vector<Type>::operator==(const Vector& other) const
{
	if (this == &other) { return true; }

	if (size != other.size) { return false; }

	for (Type* it0 = array, *it1 = other.array, *end = array + size; it0 != end; ++it0, ++it1)
	{
		if (*it0 != *it0) { return false; }
	}

	return true;
}

template<class Type>
inline bool Vector<Type>::operator!=(const Vector& other) const
{
	if (this == &other) { return false; }

	if (size != other.size) { return true; }

	for (Type* it0 = array, *it1 = other.array, *end = array + size; it0 != end; ++it0, ++it1)
	{
		if (*it0 != *it0) { return true; }
	}

	return false;
}

