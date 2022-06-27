#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"
#include "Memory/Memory.hpp"
#include "Containers/List.hpp"

template <typename TKey, typename TValue>
struct NH_API HashMap
{
public:
    struct NH_API Node
    {
        Node() {}
        Node(const TKey& key, const TValue& value) : key{ key }, value{ value } {}
        Node(TKey&& key, TValue&& value) : key{ key }, value{ value } {}

        bool operator==(const Node& other) { return key == other.key; }
        bool operator!=(const Node& other) { return key != other.key; }

        TKey key;
        TValue value;
    };

    struct Iterator
    {
        Iterator(List<Node>* ptr) : ptr{ ptr } {}

        List<Node>& operator* () const { return *ptr; }
        List<Node>* operator-> () { return ptr; }

        Iterator& operator++ () { ++ptr; return *this; }
        Iterator operator++ (int)
        {
            Iterator temp = *this;
            ++ptr;
            return temp;
        }

        Iterator& operator-- () { --ptr; return *this; }
        Iterator operator-- (int)
        {
            Iterator temp = *this;
            --ptr;
            return temp;
        }

        Iterator operator+(int i)
        {
            Iterator temp = *this;
            temp += i;
            return temp;
        }

        Iterator operator-(int i)
        {
            Iterator temp = *this;
            temp -= i;
            return temp;
        }

        Iterator& operator+=(int i)
        {
            ptr += i;
            return *this;
        }

        Iterator& operator-=(int i)
        {
            ptr -= i;
            return *this;
        }

        NH_API friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
        NH_API friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
        NH_API friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
        NH_API friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
        NH_API friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
        NH_API friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

    private:
        List<Node>* ptr;
    };

public:
    HashMap() : size{ 0 }, buckets{ nullptr } {}
    HashMap(U64 size, TValue invalid);
    HashMap(const HashMap& other);
    HashMap(HashMap&& other);
    ~HashMap();
    void Destroy();

    HashMap& operator=(const HashMap& other);
    HashMap& operator=(HashMap&& other);

    void Insert(const TKey& key, const TValue& value);
    void Insert(TKey&& key, TValue&& value);
    TValue&& Remove(const TKey& key);

    const TValue& Get(const TKey& key) const;
    TValue& Get(const TKey& key);
    const TValue& operator[](const TKey& key) const;
    TValue& operator[](const TKey& key);

    void Empty();

    Iterator begin() { return Iterator{ buckets }; }
    Iterator end() { return Iterator{ &buckets[size] }; }

private:
    U64 size;
    List<Node>* buckets;
    TValue invalid;
};

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>::HashMap(U64 size, TValue invalid) : size{ size }, invalid{ invalid }
{
    buckets = (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
}

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>::HashMap(const HashMap& other) : size{ other.size }
{
    buckets = (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(buckets, other.buckets, sizeof(List<Node>) * size);
}

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>::HashMap(HashMap&& other) : size{ other.size }, buckets{ other.buckets }, invalid{ invalid }
{
    other.size = 0;
    other.buckets = nullptr;
}

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>::~HashMap()
{
    Empty();
    Memory::Free(buckets, sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
}

template<typename TKey, typename TValue>
inline void HashMap<TKey, TValue>::Destroy()
{
    Empty();
    Memory::Free(buckets, sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
}

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>& HashMap<TKey, TValue>::operator=(const HashMap& other)
{
    if (buckets) { Destroy(); }

    size = other.size;
    buckets = (List<Node>*)Memory::Allocate(sizeof(List<Node>) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Copy(buckets, other.buckets, sizeof(List<Node>) * size);

    return *this;
}

template<typename TKey, typename TValue>
inline HashMap<TKey, TValue>& HashMap<TKey, TValue>::operator=(HashMap&& other)
{
    if (buckets) { Destroy(); }

    size = other.size;
    buckets = other.buckets;
    invalid = other.invalid;

    other.size = 0;
    other.buckets = nullptr;

    return *this;
}

template<typename TKey, typename TValue>
inline void HashMap<TKey, TValue>::Insert(const TKey& key, const TValue& value)
{
    buckets[Math::Hash(key, size)].PushFront(Node(key, value));
}

template<typename TKey, typename TValue>
inline void HashMap<TKey, TValue>::Insert(TKey&& key, TValue&& value)
{
    buckets[Math::Hash(key, size)].PushFront(Node(key, value));
}

template<typename TKey, typename TValue>
inline TValue&& HashMap<TKey, TValue>::Remove(const TKey& key)
{
    List<Node>& list = buckets[Math::Hash(key, size)];

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if (it->key == key) { return list.Remove(it).value; }
    }

    return Move(invalid);
}

template<typename TKey, typename TValue>
inline const TValue& HashMap<TKey, TValue>::Get(const TKey& key) const
{
    List<Node>& list = buckets[Math::Hash(key, size)];

    for (Node& n : list)
    {
        if (n.key == key) { return n.value; }
    }

    return invalid;
}

template<typename TKey, typename TValue>
inline TValue& HashMap<TKey, TValue>::Get(const TKey& key)
{
    List<Node>& list = buckets[Math::Hash(key, size)];

    for (Node& n : list)
    {
        if (n.key == key) { return n.value; }
    }

    return invalid;
}

template<typename TKey, typename TValue>
inline const TValue& HashMap<TKey, TValue>::operator[](const TKey& key) const
{
    List<Node>& list = buckets[Math::Hash(key, size)];

    for (Node& n : list)
    {
        if (n.key == key) { return n.value; }
    }

    return invalid;
}

template<typename TKey, typename TValue>
inline TValue& HashMap<TKey, TValue>::operator[](const TKey& key)
{
    List<Node>& list = buckets[Math::Hash(key, size)];

    for (Node& n : list)
    {
        if (n.key == key) { return n.value; }
    }

    return invalid;
}

template<typename TKey, typename TValue>
inline void HashMap<TKey, TValue>::Empty()
{
    List<Node>* ptr = buckets;
    for (U64 i = 0; i < size; ++i, ++ptr)
    {
        ptr->Clear();
    }
}
