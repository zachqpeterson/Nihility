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
        Node(const TKey& key, const TValue& value) : key{ key }, value{ value }, left{ nullptr }, right{ nullptr } {}

        TKey key;
        TValue value;
        Node* left;
        Node* right;
    };

public:
    NH_API Map();
    NH_API Map(const Map& other);
    NH_API Map(Map&& other) noexcept;
    NH_API ~Map();

    NH_API Map& operator=(const Map& other);
    NH_API Map& operator=(Map&& other) noexcept;

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
    NH_API const U64 Count(const TKey& key) const;

private:
    U64 size;
    Node* root;
};

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map() : size{ 0 }, root{ nullptr } {}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(const Map<TKey, TValue>& other) : size{ other.size }
{
    //TODO: Recursive
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Map<TKey, TValue>&& other) noexcept : size{ other.size }, root{ other.root }
{
    other.size = 0;
    other.root = nullptr;
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

    //TODO: Recursive
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>& Map<TKey, TValue>::operator=(Map<TKey, TValue>&& other) noexcept
{
    size = other.size;
    root = other.root;

    other.size = 0;
    other.root = nullptr;
}

template<typename TKey, typename TValue>
inline TValue& Map<TKey, TValue>::At(const TKey& key)
{
    ASSERT_DEBUG_MSG(root, "Cannot search an empty map!");

    Node* node = root;

    while (node && node->key != key)
    {
        node = (key < node->key) ? node->left : node->right;
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
        node = (key < node->key) ? node->left : node->right;
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
        node = (key < node->key) ? node->left : node->right;
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
        node = (key < node->key) ? node->left : node->right;
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::Clear()
{
    //TODO: Recursive
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::Insert(const TKey& key, const TValue& value)
{
    Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->key = key;
    newNode->value = value;
    newNode->left = nullptr;
    newNode->right = nullptr;

    ++size;
    if (root == nullptr)
    {
        root = newNode;
        return;
    }

    Node* node = root;

    while (node)
    {
        if (key < node->key)
        {
            if (node->left == nullptr)
            {
                node->left = newNode;
                break;
            }

            node = node->left;
        }
        else
        {
            if (node->right == nullptr)
            {
                node->right = newNode;
                break;
            }

            node = node->right;
        }
    }
}

template<typename TKey, typename TValue>
inline void Map<TKey, TValue>::InsertAssign(const TKey& key, const TValue& value)
{
    if (root == nullptr)
    {
        ++size;
        Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
        newNode->key = key;
        newNode->value = value;
        newNode->left = nullptr;
        newNode->right = nullptr;
        root = newNode;
        return;
    }

    Node* node = root;

    while (node)
    {
        if (key == node->key)
        {
            node->value = value;
        }

        if (key < node->key)
        {
            if (node->left == nullptr)
            {
                ++size;
                Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
                newNode->key = key;
                newNode->value = value;
                newNode->left = nullptr;
                newNode->right = nullptr;
                node->left = newNode;
                break;
            }

            node = node->left;
        }
        else
        {
            if (node->right == nullptr)
            {
                ++size;
                Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
                newNode->key = key;
                newNode->value = value;
                newNode->left = nullptr;
                newNode->right = nullptr;
                node->right = newNode;
                break;
            }

            node = node->right;
        }
    }
}

template<typename TKey, typename TValue>
inline TValue& Map<TKey, TValue>::InsertGet(const TKey& key)
{
    if (root == nullptr)
    {
        ++size;
        Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
        newNode->key = key;
        newNode->left = nullptr;
        newNode->right = nullptr;
        root = newNode;
        return newNode->value;
    }

    Node* node = root;

    while (node)
    {
        if (key == node->key)
        {
            return node->value;
        }

        if (key < node->key)
        {
            if (node->left == nullptr)
            {
                ++size;
                Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
                newNode->key = key;
                newNode->left = nullptr;
                newNode->right = nullptr;
                node->left = newNode;
                return newNode->value;
            }

            node = node->left;
        }
        else
        {
            if (node->right == nullptr)
            {
                ++size;
                Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
                newNode->key = key;
                newNode->left = nullptr;
                newNode->right = nullptr;
                return newNode->value;
            }

            node = node->right;
        }
    }

    return node->value;
}

template<typename TKey, typename TValue>
inline TValue&& Map<TKey, TValue>::Remove(const TKey& key)
{
    if (root)
    {
        Node* node = root;

        while (node && node->key != key)
        {
            node = (key < node->key) ? node->left : node->right;
        }

        if (node)
        {
            --size;
            TValue value = node->value;
            Memory::Free(node, sizeof(Node), MEMORY_TAG_DATA_STRUCT);
            return move(value);
        }
    }

    return {};
}

template<typename TKey, typename TValue>
inline bool Map<TKey, TValue>::Contains(const TValue& value) const
{
    //TODO: Recursive
}

template<typename TKey, typename TValue>
inline bool Map<TKey, TValue>::Contains(const TKey& key) const
{
    if (root)
    {
        Node* node = root;

        while (node && node->key != key)
        {
            node = (key < node->key) ? node->left : node->right;
        }

        return true;
    }

    return false;
}

template<typename TKey, typename TValue>
inline const U64 Map<TKey, TValue>::Count(const TKey& key) const
{
    //TODO: Recursive
}
