#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"

inline U32 Hash(const char* str, U64 size)
{
    U32 hash = 0;
    while (*str)
    {
        hash = hash * 101 + *str;
        ++str;
    }
    return hash % size;
}

inline U32 Hash(U32 i, U64 size)
{
    i = ((i >> 16) ^ i) * 0x45d9f3b;
    i = ((i >> 16) ^ i) * 0x45d9f3b;
    i = (i >> 16) ^ i;
    return i % size;
}

inline U32 Hash(U64 i, U64 size)
{
    i = (i ^ (i >> 30)) * 0xbf58476d1ce4e5b9ull;
    i = (i ^ (i >> 27)) * 0x94d049bb133111ebull;
    i = i ^ (i >> 31);
    return (U32)i % size;
}

template <typename TKey, typename TValue>
struct Hashtable
{
public:
    NH_API Hashtable(U64 size);
    NH_API Hashtable(const Hashtable& other);
    NH_API Hashtable(Hashtable&& other);
    NH_API ~Hashtable();
    NH_API void Destroy();
    NH_API Hashtable& operator=(const Hashtable& other);
    NH_API Hashtable& operator=(Hashtable&& other);

    NH_API void Set(const TKey& key, const TValue& value);
    NH_API TValue& Get(const TKey& key);
    NH_API const TValue& Get(const TKey& key) const;

    NH_API void Fill(const TValue& value);

    NH_API TValue& operator[](const TKey& key);
    NH_API const TValue& operator[](const TKey& key) const;

private:
    U64 size;
    TValue* memory;
};

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>::Hashtable(U64 size) : size{ size }
{
    memory = Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Zero(memory, sizeof(TValue) * size);
}

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>::Hashtable(const Hashtable& other) : size{ other.size }
{
    memory = Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(memory, other.memory, size);
}

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>::Hashtable(Hashtable&& other) : size{ other.size }, memory{ other.memory }
{
    other.memory = nullptr;
    other.size = 0;
}

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>& Hashtable<TKey, TValue>::operator=(const Hashtable& other)
{
    size = other.size;
    memory = Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(memory, other.memory, size);
}

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>& Hashtable<TKey, TValue>::operator=(Hashtable&& other)
{
    size = other.size;
    memory = other.memory;
    other.memory = nullptr;
    other.size = 0;
}

template<typename TKey, typename TValue>
inline Hashtable<TKey, TValue>::~Hashtable()
{
    if (memory)
    {
        Memory::Free(memory, sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
        memory = nullptr;
    }

    memory = nullptr;
}

template<typename TKey, typename TValue>
inline void Hashtable<TKey, TValue>::Destroy()
{
    if (memory)
    {
        Memory::Free(memory, sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
        memory = nullptr;
    }

    memory = nullptr;
}

template<typename TKey, typename TValue>
inline void Hashtable<TKey, TValue>::Set(const TKey& key, const TValue& value)
{
    (&memory)[Hash(key, size)] = value;
}

template<typename TKey, typename TValue>
inline TValue& Hashtable<TKey, TValue>::Get(const TKey& key)
{
    return (&memory)[Hash(key, size)];
}

template<typename TKey, typename TValue>
inline const TValue& Hashtable<TKey, TValue>::Get(const TKey& key) const
{
    return (&memory)[Hash(key, size)];
}

template<typename TKey, typename TValue>
inline TValue& Hashtable<TKey, TValue>::operator[](const TKey& key)
{
    return (&memory)[Hash(key, size)];
}

template<typename TKey, typename TValue>
inline const TValue& Hashtable<TKey, TValue>::operator[](const TKey& key) const
{
    return (&memory)[Hash(key, size)];
}

template<typename TKey, typename TValue>
inline void Hashtable<TKey, TValue>::Fill(const TValue& value)
{
    for(U64 i = 0; i < size; ++i) { (&memory)[i] = value; }
}