#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"
#include <map>

template<typename TKey, typename TValue>
struct Map
{
    struct Node
    {
        Node(const TKey& key, const TValue& value) : key{ key }, value{ value }, left{ nullptr }, right{ nullptr } { }
        ~Node() { left = nullptr; right = nullptr; } //TODO: Deallocate key and value

        void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
        void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

        TKey key;
        TValue value;
        Node* left;
        Node* right;
    };

public:
    NH_API Map();
    NH_API Map(const Map& other);
    NH_API Map(Map&& other);
    NH_API ~Map();

    NH_API Map& operator=(const Map& other);
    NH_API Map& operator=(Map&& other);

    NH_API TValue& At(const TKey& key);
    NH_API const TValue& At(const TKey& key) const;
    NH_API TValue& operator[](const TKey& key);
    NH_API const TValue& operator[](const TKey& key) const;

    NH_API const bool Empty() const { return !size; }
    NH_API const U64& Size() const { return size; }

    NH_API void Clear();
    NH_API void Insert(const TKey& key, const TValue& value);
    NH_API void InsertAssign(const TKey& key, const TValue& value);
    NH_API TValue& InsertGet(const TKey& key);
    NH_API TValue&& Remove(const TKey& key);

    NH_API bool Contains(const TValue& value) const;
    NH_API bool Contains(const TKey& key) const;
    NH_API const U64& Count(const TKey& key) const { return size; }

private:
    Node* root;
    U64 size;
};

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map() : root{ nullptr }, size{ 0 } {}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(const Map<TKey, TValue>& other) : size{ 0 }
{
    //TODO: Use queue
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Map<TKey, TValue>&& other) : root{ other.root }, size{ other.size }
{
    other.root = nullptr;
    other.size = 0;
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::~Map()
{
    Clear();
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>& Map<TKey, TValue>::operator=(const Map<TKey, TValue>& other)
{
    size = other.size;
    //TODO: Use queue
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>& Map<TKey, TValue>::operator=(Map<TKey, TValue>&& other)
{
    root = other.root;
    size = other.size;

    other.root = nullptr;
    other.size = 0;
}

template<typename TKey, typename TValue>
inline TValue& Map<TKey, TValue>::At(const TKey& key)
{
    ASSERT_DEBUG_MSG(root, "Cannot search an empty map!");

    Node* node = root;

    while (node && node->key != key)
    {
        node = (&node->left)[key > node->key];
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::At(const TKey& key) const
{
    ASSERT_DEBUG_MSG(root, "Cannot search an empty map!");

    Node* node = root;

    while (node && node->key != key)
    {
        node = (&node->left)[key > node->key];
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline TValue& Map<TKey, TValue>::operator[](const TKey& key)
{
    ASSERT_DEBUG_MSG(root, "Cannot search an empty map!");

    Node* node = root;

    while (node && node->key != key)
    {
        node = (&node->left)[key > node->key];
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::operator[](const TKey& key) const
{
    ASSERT_DEBUG_MSG(root, "Cannot search an empty map!");

    Node* node = root;

    while (node && node->key != key)
    {
        node = (&node->left)[key > node->key];
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::Clear()
{
    //TODO: Use queue
    size = 0;
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::Insert(const TKey& key, const TValue& value)
{
    if (root == nullptr)
    {
        ++size;
        root = new Node(key, value);
        return;
    }

    Node* node = root;

    while (node && node->key != key)
    {
        Node** next = (&node->left) + (key > node->key);

        if (!*next)
        {
            ++size;
            *next = new Node(key, value);
            return;
        }

        node = *next;
    }
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::InsertAssign(const TKey& key, const TValue& value)
{
    if (root == nullptr)
    {
        ++size;
        root = new Node(key, value);
        return;
    }

    Node* node = root;

    while (node && node->key != key)
    {
        Node** next = (&node->left) + (key > node->key);

        if (!*next)
        {
            ++size;
            *next = new Node(key, value);
            return;
        }

        node = *next;
    }

    if (node)
    {
        node->value = value;
    }
}

template<typename TKey, typename TValue>
inline TValue& Map<TKey, TValue>::InsertGet(const TKey& key)
{
    if (root == nullptr)
    {
        ++size;
        root = new Node(key, TValue{});
        return root->value;
    }

    Node* node = root;

    while (node && node->key != key)
    {
        Node** next = (&node->left) + (key > node->key);

        if (!*next)
        {
            ++size;
            *next = new Node(key, TValue{});
        }

        node = *next;
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline TValue&& Map<TKey, TValue>::Remove(const TKey& key)
{
    if (root)
    {
        Node* parent = root;
        Node* node = root;

        bool right = false;

        while (node && node->value != value)
        {
            parent = node;
            node = (&node->left)[right = value > node->value];
        }

        if (node)
        {
            Node* replaceParent = node;
            Node* replace = node->left;

            while (replace && replace->right)
            {
                replaceParent = replace;
                replace = replace->right;
            }

            if (replace)
            {
                if (replaceParent != node) { replaceParent->right = nullptr; }
                replace->right = node->right;
                if (replace != node->left) { replace->left = node->left; }
            }

            if (node->value == root->value) { root = replace; }
            else { *((&parent->left) + right) = replace; }
            TValue t = node->value;
            --size;
            delete node;
            return Move(t);
        }
    }

    return {};
}

template<typename TKey, typename TValue>
inline bool Map<TKey, TValue>::Contains(const TValue& value) const
{
    //TODO: Use queue
}

template<typename TKey, typename TValue>
inline bool Map<TKey, TValue>::Contains(const TKey& key) const
{
    Node* node = root;

    while (node && node->key != key)
    {
        node = (&node->left)[key > node->key];
    }

    return node;
}