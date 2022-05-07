#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct Vector
{
public:
    struct Iterator
    {
        Iterator(T* ptr) : ptr{ ptr } {}

        T& operator* () const { return *ptr; }
        T* operator-> () { return ptr; }

        Iterator& operator++ () { ++ptr; return *this; }
        Iterator operator++ (int)
        {
            iterator temp = *this;
            ++this->ptr;
            return temp;
        }

        Iterator& operator-- () { --ptr; return *this; }
        Iterator operator-- (int)
        {
            iterator temp = *this;
            --this->ptr;
            return temp;
        }

        //TODO: +=/-=

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
        friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
        friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
        friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
        friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

    private:
        T* ptr;
    };

public:
    Vector(U64 size = 0, const T& value = {});
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;
    ~Vector();

    Vector& operator=(const Vector& other);
    Vector& operator=(Vector&& other)noexcept;

    void Push(const T& value);
    void Push(T&& value) noexcept;
    T&& Pop() noexcept;
    void Insert(const T& value, U64 index);
    void Insert(T&& value, U64 index) noexcept;
    T&& Remove(U64 index)noexcept;
    void Resize(U64 size);
    void Reserve(U64 capacity);
    const U64 Find(const T& value);
    const U64 Find(const T& value) const;
    void Clear();
    T* Data();
    const T* Data() const;
    T& Front();
    const T& Front() const;
    T& Back();
    const T& Back() const;
    const U64& Size() const;
    const U64& Capacity() const;

    T& operator[](U64 i);
    const T& operator[](U64 i) const;

    Iterator begin() { return Iterator{ array }; }
    Iterator end() { return Iterator{ &array[size] }; }

private:
    U64 size;
    U64 capacity;
    T* array;
};

template<typename T>
inline Vector<T>::Vector(U64 size, const T& value) : size{ size }, capacity{ size }
{
    array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

    for (int i = 0; i < size; ++i)
    {
        array[i] = value;
    }
}

template<typename T>
inline Vector<T>::Vector(const Vector<T>& other) : size{ other.size }, capacity{ other.capacity }
{
    array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

    Memory::CopyMemory(array, other.array, size);
}

template<typename T>
inline Vector<T>::Vector(Vector<T>&& other) noexcept : size{ other.size }, capacity{ other.capacity }, array{ other.array }
{
    other.size = 0;
    other.capacity = 0;
    other.array = nullptr;
}

template<typename T>
inline Vector<T>::~Vector()
{
    if (array)
    {
        Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
    }

    array = nullptr;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
    if (array)
    {
        Memory::Free(array, sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);
    }

    size = other.size;
    capacity = other.capacity;
    array = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

    Memory::CopyMemory(array, other.array, size);

    return *this;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector<T>&& other)
{
    size = other.size;
    capacity = other.capacity;
    array = other.array;
    other.size = 0;
    other.capacity = 0;
    other.array = nullptr;

    return *this;
}

template<typename T>
inline void Vector<T>::Push(const T& value)
{
    if (size == capacity)
    {
        Reserve(++capacity * 2);
    }

    elements[size] = value;
    ++size;
}

template<typename T>
inline void Vector<T>::Push(T&& value) noexcept
{
    if (size == capacity)
    {
        Reserve(++capacity * 2);
    }

    elements[size] = move(value);
    ++size;
}

template<typename T>
inline T&& Vector<T>::Pop() noexcept
{
    ASSERT_DEBUG_MSG(size, "Can't pop on a vector of size 0!");
    return move(elements[--size]);
}

template<typename T>
inline void Vector<T>::Insert(const T& value, U64 index)
{
    ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");

    if (size == capacity)
    {
        Reserve(++capacity * 2);
    }

    ++size;

    Memory::CopyMemory(array + index + 1, array + index, size - index);
    array[index] = data;
}

template<typename T>
inline void Vector<T>::Insert(T&& value, U64 index) noexcept
{
    ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");

    if (size == capacity)
    {
        Reserve(++capacity * 2);
    }

    ++size;

    Memory::CopyMemory(array + index + 1, array + index, size - index);
    array[index] = value;
}

template<typename T>
inline T&& Vector<T>::Remove(U64 index)
{
    ASSERT_DEBUG_MSG(index < size, "Can't index past the size of a vector!");
    T value = array[index];

    Memory::CopyMemory(array + index, array + index + 1, size - index - 1);
    --size;

    return move(value);
}


template<typename T>
inline void Vector<T>::Resize(U64 size)
{
    if (size > capacity)
    {
        Reserve(size);
    }

    this->size = size;
}

template<typename T>
inline void Vector<T>::Reserve(U64 capacity)
{
    T* newArray = (T*)Memory::Allocate(sizeof(T) * capacity, MEMORY_TAG_DATA_STRUCT);

    //TODO: deallocate pointers if T is a pointer type

    Memory::Free(array, sizeof(T) * this->capacity, MEMORY_TAG_DATA_STRUCT);
    array = newArray;
    this->capacity = capacity;
    size = size > capacity ? capacity : size;
}

template<typename T>
inline const U64 Vector<T>::Find(const T& value)
{
    for (int i = 0; i < size; ++i)
    {
        if (array[size] == value)
        {
            return i;
        }
    }

    return -1;
}

template<typename T>
inline const U64 Vector<T>::Find(const T& value) const
{
    for (int i = 0; i < size; ++i)
    {
        if (array[size] == value)
        {
            return i;
        }
    }

    return -1;
}

template<typename T>
inline void Vector<T>::Clear()
{
    size = 0;
}

template<typename T>
inline T* Vector<T>::Data()
{
    return array;
}

template<typename T>
inline const T* Vector<T>::Data() const
{
    return array;
}

template<typename T>
inline T& Vector<T>::Front()
{
    return array[0];
}

template<typename T>
inline const T& Vector<T>::Front() const
{
    return array[0];
}

template<typename T>
inline T& Vector<T>::Back()
{
    return array[size - 1];
}

template<typename T>
inline const T& Vector<T>::Back() const
{
    return array[size - 1];
}

template<typename T>
inline const U64& Vector<T>::Size() const
{
    return size;
}

template<typename T>
inline const U64& Vector<T>::Capacity() const
{
    return capacity;
}

template<typename T>
inline T& Vector<T>::operator[](U64 i)
{
    ASSERT_DEBUG_MSG(i < size, "Can't index past the size of a vector!");
    return array[i];
}

template<typename T>
inline const T& Vector<T>::operator[](U64 i) const
{
    ASSERT_DEBUG_MSG(i < size, "Can't index past the size of a vector!");
    return array[i];
}
