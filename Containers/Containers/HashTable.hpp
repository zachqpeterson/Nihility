#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"

//TODO: Use quadratic probing
template <typename TKey, typename TValue>
struct  HashTable
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
    TValue* memory;
};