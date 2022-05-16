#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"

inline U32 Hash(const char* str)
{
    U32 hash = 0;
    while(*str)
    {
        hash = hash * 101 + *str;
        ++str;
    }
    return hash;
}

inline U32 Hash(U32 i)
{
    i = ((i >> 16) ^ i) * 0x45d9f3b;
    i = ((i >> 16) ^ i) * 0x45d9f3b;
    i = (i >> 16) ^ i;
    return i;
}

inline U32 Hash(U64 i)
{
    i = (i ^ (i >> 30)) * 0xbf58476d1ce4e5b9ull;
    i = (i ^ (i >> 27)) * 0x94d049bb133111ebull;
    i = i ^ (i >> 31);
    return (U32)i;
}

template <typename T>
struct Hashtable
{
public:
    NH_API Hashtable(U64 size);

private:
    U64 size;
    T* memory;
};

template<typename T>
inline Hashtable<T>::Hashtable(U64 size) : size{size}
{
    memory = Memory::Allocate(sizeof(T) * size, MEMORY_TAG_DATA_STRUCT);
}