#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

template <typename TKey, typename TValue> //TODO: Only use key
struct HashTable //TODO: Rename to HashSet
{
public:
    HashTable();
    HashTable(U64 size);
    HashTable(const HashTable& other);
    HashTable(HashTable&& other);
    ~HashTable();
    void Destroy();
    HashTable& operator=(const HashTable& other);
    HashTable& operator=(HashTable&& other);

    void Set(const TKey& key, const TValue& value);
    TValue& Get(const TKey& key);
    const TValue& Get(const TKey& key) const;

    void Fill(const TValue& value);

    TValue& operator[](const TKey& key);
    const TValue& operator[](const TKey& key) const;

private:
    U64 size;
    TValue* memory; //TODO: Use buckets
};

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>::HashTable() : size{ 0 }, memory{ nullptr } { }

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>::HashTable(U64 size) : size{ size }
{
    memory = (TValue*)Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Zero(memory, sizeof(TValue) * size);
}

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>::HashTable(const HashTable& other) : size{ other.size }
{
    memory = (TValue*)Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(memory, other.memory, size);
}

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>::HashTable(HashTable&& other) : size{ other.size }, memory{ other.memory }
{
    other.memory = nullptr;
    other.size = 0;
}

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>& HashTable<TKey, TValue>::operator=(const HashTable& other)
{
    size = other.size;
    memory = (TValue*)Memory::Allocate(sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(memory, other.memory, size);

    return *this;
}

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>& HashTable<TKey, TValue>::operator=(HashTable&& other)
{
    size = other.size;
    memory = other.memory;
    other.memory = nullptr;
    other.size = 0;

    return *this;
}

template<typename TKey, typename TValue>
inline HashTable<TKey, TValue>::~HashTable()
{
    if (memory)
    {
        Memory::Free(memory, sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
        memory = nullptr;
    }

    memory = nullptr;
}

template<typename TKey, typename TValue>
inline void HashTable<TKey, TValue>::Destroy()
{
    if (memory)
    {
        Memory::Free(memory, sizeof(TValue) * size, MEMORY_TAG_DATA_STRUCT);
        memory = nullptr;
    }

    memory = nullptr;
}

template<typename TKey, typename TValue>
inline void HashTable<TKey, TValue>::Set(const TKey& key, const TValue& value)
{
    memory[Math::Hash(key, size)] = value;
}

template<typename TKey, typename TValue>
inline TValue& HashTable<TKey, TValue>::Get(const TKey& key)
{
    return memory[Math::Hash(key, size)];
}

template<typename TKey, typename TValue>
inline const TValue& HashTable<TKey, TValue>::Get(const TKey& key) const
{
    return memory[Math::Hash(key, size)];
}

template<typename TKey, typename TValue>
inline TValue& HashTable<TKey, TValue>::operator[](const TKey& key)
{
    return memory[Math::Hash(key, size)];
}

template<typename TKey, typename TValue>
inline const TValue& HashTable<TKey, TValue>::operator[](const TKey& key) const
{
    return memory[Math::Hash(key, size)];
}

template<typename TKey, typename TValue>
inline void HashTable<TKey, TValue>::Fill(const TValue& value)
{
    for (U64 i = 0; i < size; ++i) { memory[i] = value; }
}