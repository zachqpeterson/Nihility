#pragma once

#include "ContainerDefines.hpp"

#include "Memory\Memory.hpp"

/*
* TODO: Variatic Templates
*	Insert
*	Merge
*	Add
*
* TODO: Sorting
*	Comparison Predicate
*
* TODO: Comparison
*	Compare two Vectors, return true if matching
*	Compare two Vectors, return count of matching
*	Compare two Vectors, return Vector<T> of matching
*	Compare two Vectors, remove matching, return count
*	Compare two Vectors, merge all unique values
*
* TODO: Insertion
*	Push value if unique
*	Insert value if unique
*
* TODO: Remove
*	Search, remove all non-unique values
*/
template<typename T>
struct Vector
{
public:
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
	Vector(U64 size, const T& value);

	/// <summary>
	/// Create a new Vector instance using an initializer list, size will equal the list size, capacity with be greater than or equal to size, creates an array of size sizeof(T) * capacity and fills it with the values in list
	/// </summary>
	/// <param name="list:">The initializer list</param>
	Vector(Initializer<T> list);

	/// <summary>
	/// Creates a new Vector instance, capacity and size will be other's, creates an array of the same size and copies other's data into it
	/// </summary>
	/// <param name="other:">Vector to copy</param>
	Vector(const Vector& other);

	/// <summary>
	/// Creates a new Vector instance, capacity and size will be other's, sets the array to other's 
	/// WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	Vector(Vector&& other) noexcept;

	/// <summary>
	/// Copies other's data into this, capacity and size will be other's, creates an array of the same size and copies other's data into it
	/// WARNING: any previous data will be lost
	/// </summary>
	/// <param name="other:">The Vector to copy</param>
	/// <returns>Reference to this</returns>
	Vector& operator=(const Vector& other);

	/// <summary>
	/// Moves other's data into this, capacity and size will be other's, sets the array to other's 
	/// WARNING: any previous data will be lost
	/// WARNING: other will be destroyed
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
	/// Checks if T is Destroyable, if it is, this will call Destroy on all objects inside
	/// </summary>
	void Cleanup();



	/// <summary>
	/// Increases size by one and puts value onto the back of array, reallocates array if it's too small
	/// </summary>
	/// <param name="value:">The value to put into array</param>
	T& Push(const T& value);

	/// <summary>
	/// Increases size by one and moves value onto the back of array, reallocates array if it's too small
	/// </summary>
	/// <param name="value:">The value to move into array</param>
	T& Push(T&& value) noexcept;

	/// <summary>
	/// Decreases the size by one
	/// </summary>
	void Pop();

	/// <summary>
	/// Decreases the size by one and copies what was in the back of array to value
	/// </summary>
	/// <param name="value:">The value to copy to</param>
	void Pop(T& value);

	/// <summary>
	/// Decreases the size by one and moves what was in the back of array to value
	/// </summary>
	/// <param name="value:">The value to move to</param>
	void Pop(T&& value) noexcept;

	/// <summary>
	/// Inserts value into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to put value</param>
	/// <param name="value:">The value to copy</param>
	template<Unsigned Type> void Insert(Type index, const T& value);

	/// <summary>
	/// Inserts value into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to put value</param>
	/// <param name="value:">The value to move</param>
	template<Unsigned Type> void Insert(Type index, T&& value) noexcept;

	/// <summary>
	/// Copies other and inserts it into index, moves values at and past index over, reallocates array if it's too small
	/// </summary>
	/// <param name="index:">The index to copy other into</param>
	/// <param name="other:">The Vector to copy</param>
	template<Unsigned Type> void Insert(Type index, const Vector& other);

	/// <summary>
	/// Moves other and inserts it into index, moves values at and past index over, reallocates array if it's too small
	/// WARNING: other will be destroyed
	/// </summary>
	/// <param name="index:">The index to move other into</param>
	/// <param name="other:">The Vector to move</param>
	template<Unsigned Type> void Insert(Type index, Vector&& other) noexcept;

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
	void Remove(U64 index, T& value);

	/// <summary>
	/// Moves value at index into value, moves values past index to index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	/// <param name="value:">The value to move to</param>
	void Remove(U64 index, T&& value) noexcept;

	/// <summary>
	/// Removes the value at index by swaping it with the last index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	void RemoveSwap(U64 index);

	/// <summary>
	/// Copies value at index into value, removes the value at index by swaping it with the last index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	/// <param name="value:">The value to copy to</param>
	void RemoveSwap(U64 index, T& value);

	/// <summary>
	/// Moves value at index into value, removes the value at index by swaping it with the last index
	/// </summary>
	/// <param name="index:">The index to remove</param>
	/// <param name="value:">The value to move to</param>
	void RemoveSwap(U64 index, T&& value) noexcept;

	/// <summary>
	/// Moves values at and past index1 to index0
	/// </summary>
	/// <param name="index0:">The beginning of the erasure, inclusive</param>
	/// <param name="index1:">The end of the erasure, exclusive</param>
	void Erase(U64 index0, U64 index1);

	/// <summary>
	/// Copies values to be erased into other, moves values at and past index1 to index0
	/// WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="index0:">The beginning of the erasure, inclusive</param>
	/// <param name="index1:">The end of the erasure, exclusive</param>
	/// <param name="other:">The Vector to copy to</param>
	void Erase(U64 index0, U64 index1, Vector& other);



	/// <summary>
	/// Splits the array at index, copies data at and past index into other
	/// WARNING: any previous data in other will be lost
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
	/// WARNING: other will be destroyed
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
	/// WARNING: other will be destroyed
	/// </summary>
	/// <param name="other:">The Vector to move</param>
	/// <returns>Reference to this</returns>
	Vector& operator+=(Vector&& other) noexcept;



	/// <summary>
	/// Searches array, finds all values that satisfy predicate, fill other with those values
	/// WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <param name="other:">A Vector to fill with values</param>
	template<FunctionPtr Predicate> void SearchFor(Predicate predicate, Vector& other);

	/// <summary>
	/// Searches array, finds indices of all values that satisfy predicate, fill other with those indices
	/// WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <param name="other:">A Vector to fill with indices</param>
	template<FunctionPtr Predicate> void SearchForIndices(Predicate predicate, Vector<U64>& other);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <returns>The count of values that satisfy predicate</returns>
	template<FunctionPtr Predicate> U64 SearchCount(Predicate predicate);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate, removes them from array
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <returns>The count of values that satisfy predicate</returns>
	template<FunctionPtr Predicate> U64 RemoveAll(Predicate predicate);

	/// <summary>
	/// Searches array, finds all values that satisfy predicate, removes them from array and puts them into other
	/// WARNING: any previous data in other will be lost
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <param name="other:">A Vector to fill with values</param>
	template<FunctionPtr Predicate> void RemoveAll(Predicate predicate, Vector& other);

	/// <summary>
	/// Finds the first value that satisfies the predicate, return true if one exists
	/// </summary>
	/// <param name="predicate:">A function to evaluate values: bool pred(const T&amp; value)</param>
	/// <param name="value:">A reference to return the found value</param>
	/// <returns>true if a value exists, false otherwise</returns>
	template<FunctionPtr Predicate> T* Find(Predicate predicate);



	/// <summary>
	/// Inserts a value based on a predicate
	/// </summary>
	/// <param name="predicate:">A function to compare values: bool pred(const T&amp; a, const T&amp; b)</param>
	/// <param name="value:">The value to insert</param>
	/// <returns>The index the value was inserted at</returns>
	template<FunctionPtr Predicate> U64 SortedInsert(Predicate predicate, const T& value);

	/// <summary>
	/// Inserts a value based on a predicate
	/// </summary>
	/// <param name="predicate:">A function to compare values: bool pred(const T&amp; a, const T&amp; b)</param>
	/// <param name="value:">The value to insert</param>
	/// <returns>The index the value was inserted at</returns>
	template<FunctionPtr Predicate> U64 SortedInsert(Predicate predicate, T&& value) noexcept;



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
	void Resize(U64 size, const T& value);

	/// <summary>
	/// Sets size to zero
	/// </summary>
	void Clear() { size = 0; }



	/// <summary>
	/// Searches array for value
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>true if value is contained within array, false otherwise</returns>
	bool Contains(const T& value) const;

	/// <summary>
	/// Counts the reoccurrences of value in array
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>The number of reoccurrences of value</returns>
	U64 Count(const T& value) const;

	/// <summary>
	/// Finds the first index of value
	/// </summary>
	/// <param name="value:">The value to search for</param>
	/// <returns>The index of value, if it doesn't find value, U64_MAX</returns>
	U64 Find(const T& value) const;



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
	const T* Data() const { return array; }

	/// <summary></summary>
	/// <returns>array</returns>
	T* Data() { return array; }



	/// <summary></summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index (const)</returns>
	const T& operator[](U64 i) const { return array[i]; }

	/// <summary></summary>
	/// <param name="i:">Index</param>
	/// <returns>The value at index</returns>
	T& operator[](U64 i) { return array[i]; }

	/// <summary>
	/// Gets the value at the front of array (index 0)
	/// </summary>
	/// <returns>The value at the front of array</returns>
	T& Front() { return *array; }

	/// <summary>
	/// Gets the value at the front of array (index 0)
	/// </summary>
	/// <returns>The value at the front of array (const)</returns>
	const T& Front() const { return *array; }

	/// <summary>
	/// Gets the value at the back of array (index size - 1)
	/// </summary>
	/// <returns>The value at the back of array</returns>
	T& Back() { return array[size - 1]; }

	/// <summary>
	/// Gets the value at the back of array (index size - 1)
	/// </summary>
	/// <returns>The value at the back of array (const)</returns>
	const T& Back() const { return array[size - 1]; }



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
	T* begin() { return array; }

	/// <summary></summary>
	/// <returns>The end of array as an iterator</returns>
	T* end() { return array + size; }

	/// <summary></summary>
	/// <returns>The beginning of array as an iterator (const)</returns>
	const T* begin() const { return array; }

	/// <summary></summary>
	/// <returns>The end of array as an iterator (const)</returns>
	const T* end() const { return array + size; }

private:

	/// <summary>
	/// The count of values inside array
	/// </summary>
	U64 size{ 0 };

	/// <summary>
	/// The actual size of array
	/// </summary>
	U64 capacity{ 0 };

	/// <summary>
	/// A dynamically allocated array to store data
	/// </summary>
	T* array{ nullptr };
};

template<typename T> inline Vector<T>::Vector() {}

template<typename T> inline Vector<T>::Vector(U64 cap) { Memory::AllocateArray(&array, cap, capacity); }

template<typename T> inline Vector<T>::Vector(U64 size, const T& value) : size{ size }, capacity{ size }
{
	Memory::AllocateArray(&array, capacity, capacity);
	for (T* t = array, *end = array + size; t != end; ++t) { *t = value; }
}

template<typename T> inline Vector<T>::Vector(Initializer<T> list) : size{ list.size() }, capacity{ size }
{
	Memory::AllocateArray(&array, capacity, capacity);

	T* it1 = array;
	for (const T* it0 = list.begin(), *end = list.end(); it0 != end; ++it0, ++it1)
	{
		*it1 = *it0;
	}
}

template<typename T> inline Vector<T>::Vector(const Vector<T>& other) : size{ other.size }, capacity{ other.size }
{
	Memory::AllocateArray(&array, capacity, capacity);
	Memory::Copy(array, other.array, sizeof(T) * size);
}

template<typename T> inline Vector<T>::Vector(Vector<T>&& other) noexcept : size{ other.size }, capacity{ other.capacity }, array{ other.array }
{
	other.size = 0;
	other.capacity = 0;
	other.array = nullptr;
}

template<typename T> inline Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	size = other.size;
	if (capacity < other.size) { Memory::Reallocate(&array, size, capacity); }

	Memory::Copy(array, other.array, size * sizeof(T));

	return *this;
}

template<typename T> inline Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
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

template<typename T> inline Vector<T>::~Vector() { Destroy(); }

template<typename T> inline void Vector<T>::Destroy() { size = 0; capacity = 0; Memory::Free(&array); }

template<typename T> inline void Vector<T>::Cleanup()
{
	if constexpr (IsDestroyable<T>)
	{
		for (T* t = array, *end = array + size; t != end; ++t)
		{
			t->Destroy();
		}
	}
}

template<typename T> inline T& Vector<T>::Push(const T& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[size] = value;
	return array[size++];
}

template<typename T> inline T& Vector<T>::Push(T&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	array[size] = Move(value);
	return array[size++];
}

template<typename T> inline void Vector<T>::Pop()
{
	if (size) { --size; }
}

template<typename T> inline void Vector<T>::Pop(T& value)
{
	if (size) { value = array[--size]; }
}

template<typename T> inline void Vector<T>::Pop(T&& value) noexcept
{
	if (size) { value = Move(array[--size]); }
}

template<typename T>
template<Unsigned Type>
inline void Vector<T>::Insert(Type index, const T& value)
{
	if (size == capacity) { Reserve(capacity + 1); }

	Memory::Copy(array + index + 1, array + index, (size - index) * sizeof(T));
	array[index] = value;
	++size;
}

template<typename T>
template<Unsigned Type>
inline void Vector<T>::Insert(Type index, T&& value) noexcept
{
	if (size == capacity) { Reserve(capacity + 1); }

	Memory::Copy(array + index + 1, array + index, (size - index) * sizeof(T));
	array[index] = Move(value);
	++size;
}

template<typename T>
template<Unsigned Type>
inline void Vector<T>::Insert(Type index, const Vector<T>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + index + other.size, array + index, (size - index) * sizeof(T));
	Memory::Copy(array + index, other.array, other.size * sizeof(T));

	size += other.size;
}

template<typename T>
template<Unsigned Type>
inline void Vector<T>::Insert(Type index, Vector<T>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + index + other.size, array + index, (size - index) * sizeof(T));
	Memory::Copy(array + index, other.array, other.size * sizeof(T));
	size += other.size;

	other.Destroy();
}

template<typename T> inline void Vector<T>::Remove(U64 index)
{
	Memory::Copy(array + index, array + index + 1, (size - index) * sizeof(T));

	--size;
}

template<typename T> inline void Vector<T>::Remove(U64 index, T& value)
{
	value = array[index];
	Memory::Copy(array + index, array + index + 1, (size - index) * sizeof(T));

	--size;
}

template<typename T> inline void Vector<T>::Remove(U64 index, T&& value) noexcept
{
	value = Move(array[index]);
	Memory::Copy(array + index, array + index + 1, (size - index) * sizeof(T));

	--size;
}

template<typename T> inline void Vector<T>::RemoveSwap(U64 index)
{
	array[index] = array[--size];
}

template<typename T> inline void Vector<T>::RemoveSwap(U64 index, T& value)
{
	value = array[index];
	array[index] = array[--size];
}

template<typename T> inline void Vector<T>::RemoveSwap(U64 index, T&& value) noexcept
{
	value = Move(array[index]);
	array[index] = array[--size];
}

template<typename T> inline void Vector<T>::Erase(U64 index0, U64 index1)
{
	Memory::Copy(array + index0, array + index1, (size - index1) * sizeof(T));

	size -= index1 - index0;
}

template<typename T> inline void Vector<T>::Erase(U64 index0, U64 index1, Vector<T>& other)
{
	other.Reserve(index1 - index0);
	other.size = other.capacity;

	Memory::Copy(other.array, array + index0, (index1 - index0) * sizeof(T));
	Memory::Copy(array + index0, array + index1, (size - index1) * sizeof(T));

	size -= index1 - index0;
}

template<typename T> inline void Vector<T>::Split(U64 index, Vector<T>& other)
{
	other.Reserve(size - index);
	other.size = other.capacity;

	Memory::Copy(other.array, array + index, other.size * sizeof(T));

	size -= index;
}

template<typename T> inline void Vector<T>::Merge(const Vector<T>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + size, other.array, other.size * sizeof(T));
	size += other.size;
}

template<typename T> inline void Vector<T>::Merge(Vector<T>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + size, other.array, other.size * sizeof(T));
	size += other.size;

	other.Destroy();
}

template<typename T> inline Vector<T>& Vector<T>::operator+=(const Vector<T>& other)
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + size, other.array, other.size * sizeof(T));
	size += other.size;

	return *this;
}

template<typename T> inline Vector<T>& Vector<T>::operator+=(Vector<T>&& other) noexcept
{
	if (size + other.size > capacity) { Reserve(size + other.size); }

	Memory::Copy(array + size, other.array, other.size * sizeof(T));
	size += other.size;

	other.Destroy();

	return *this;
}

template<typename T>
template<FunctionPtr Predicate>
inline void Vector<T>::SearchFor(Predicate predicate, Vector<T>& other)
{
	other.Reserve(size);
	other.size = 0;

	for (T* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(*t)) { other.Push(*t); }
	}
}

template<typename T>
template<FunctionPtr Predicate>
inline void Vector<T>::SearchForIndices(Predicate predicate, Vector<U64>& other)
{
	other.Reserve(size);
	other.size = 0;

	U64 i = 0;
	for (T* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(*t)) { other.Push(i); }
	}
}

template<typename T>
template<FunctionPtr Predicate>
inline U64 Vector<T>::SearchCount(Predicate predicate)
{
	U64 i = 0;
	for (T* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(*t)) { ++i; }
	}

	return i;
}

template<typename T>
template<FunctionPtr Predicate>
inline U64 Vector<T>::RemoveAll(Predicate predicate)
{
	T* last = array + size;

	U64 i = 0;
	for (T* t = array, *end = array + size; t != end;)
	{
		if (predicate(*t))
		{
			++i;
			Memory::Copy(t, t + 1, sizeof(T) * (last - t - 1));
			--size;
		}
		else { ++t; }
	}

	return i;
}

template<typename T>
template<FunctionPtr Predicate>
inline void Vector<T>::RemoveAll(Predicate predicate, Vector<T>& other)
{
	T* last = array + size;

	other.Reserve(size);
	other.size = 0;

	for (T* t = array, *end = array + size; t != end;)
	{
		if (predicate(*t))
		{
			other.Push(*t);
			Memory::Copy(t, t + 1, sizeof(T) * (last - t - 1));

			--size;
		}
		else { ++t; }
	}
}

template<typename T>
template<FunctionPtr Predicate>
inline T* Vector<T>::Find(Predicate predicate)
{
	for (T* t = array, *end = array + size; t != end; ++t)
	{
		if (predicate(t)) { return t; }
	}

	return nullptr;
}

template<typename T>
template<FunctionPtr Predicate>
U64 Vector<T>::SortedInsert(Predicate predicate, const T& value)
{
	U64 i = 0;
	for (T* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(value, *t))
		{
			if (size == capacity) { Reserve(capacity + 1); }

			Memory::Copy(array + i + 1, array + i, (size - i) * sizeof(T));
			array[i] = value;
			++size;

			return i;
		}
	}

	U64 index = size;

	Push(value);

	return index;
}

template<typename T>
template<FunctionPtr Predicate>
U64 Vector<T>::SortedInsert(Predicate predicate, T&& value) noexcept
{
	U64 i = 0;
	for (T* t = array, *end = array + size; t != end; ++t, ++i)
	{
		if (predicate(value, *t))
		{
			if (size == capacity) { Reserve(capacity + 1); }

			Memory::Copy(array + i + 1, array + i, (size - i) * sizeof(T));
			array[i] = Move(value);
			++size;

			return i;
		}
	}

	U64 index = size;

	Push(Move(value));

	return index;
}

template<typename T>
inline void Vector<T>::Reserve(U64 capacity)
{
	Memory::Reallocate(&array, capacity, this->capacity);
}

template<typename T>
inline void Vector<T>::Resize(U64 size)
{
	if (size > capacity) { Reserve(size); }
	this->size = size;
}

template<typename T>
inline void Vector<T>::Resize(U64 size, const T& value)
{
	if (size > capacity) { Reserve(size); }
	this->size = size;

	for (U64 i = 0; i < size; ++i) { array[i] = value; }
}

template<typename T>
inline bool Vector<T>::Contains(const T& value) const
{
	for (T* t = array, *end = array + size; t != end; ++t)
	{
		if (*t == value) { return true; }
	}

	return false;
}

template<typename T>
inline U64 Vector<T>::Count(const T& value) const
{
	U64 count = 0;
	for (T* t = array, *end = array + size; t != end; ++t)
	{
		if (*t == value) { ++count; }
	}

	return count;
}

template<typename T>
inline U64 Vector<T>::Find(const T& value) const
{
	U64 index = 0;
	for (T* t = array; index < size; ++index, ++t)
	{
		if (*t == value) { return index; }
	}

	return -1;
}

template<typename T>
inline bool Vector<T>::operator==(const Vector& other) const
{
	if (size != other.size) { return false; }

	for (T* it0 = array, *it1 = other.array, *end = array + size; it0 != end; ++it0, ++it1)
	{
		if (*it0 != *it0) { return false; }
	}

	return true;
}

template<typename T>
inline bool Vector<T>::operator!=(const Vector& other) const
{
	if (size != other.size) { return true; }

	for (T* it0 = array, *it1 = other.array, *end = array + size; it0 != end; ++it0, ++it1)
	{
		if (*it0 != *it0) { return true; }
	}

	return false;
}