#pragma once

#include "Defines.hpp"

#include "Stack.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename TKey, typename TValue>
struct Map
{
    struct Node
    {
        Node(const TKey& key, const TValue& value) : key{ key }, value{ value }, left{ nullptr }, right{ nullptr } {}
        ~Node() { left = nullptr; right = nullptr; } //TODO: Deallocate key and value

        void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
        void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

        TKey key;
        TValue value;
        Node* left;
        Node* right;
    };

public:
    Map();
    Map(const Map& other);
    Map(Map&& other);
    ~Map();
    void Destroy();

    void* operator new(U64 size) { return Memory::Allocate(sizeof(Map), MEMORY_TAG_DATA_STRUCT); }
    void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Map), MEMORY_TAG_DATA_STRUCT); }

    Map& operator=(const Map& other);
    Map& operator=(Map&& other);

    TValue& At(const TKey& key);
    const TValue& At(const TKey& key) const;
    TValue& operator[](const TKey& key);
    const TValue& operator[](const TKey& key) const;

    const bool Empty() const { return !size; }
    const U64& Size() const { return size; }

    void Clear();
    void Insert(const TKey& key, const TValue& value);
    void InsertAssign(const TKey& key, const TValue& value);
    TValue& InsertGet(const TKey& key);
    TValue&& Remove(const TKey& key);

    bool Contains(const TValue& value) const;
    bool Contains(const TKey& key) const;
    const U64& Count(const TKey& key) const { return size; }

private:
    Node* root;
    U64 size;
};

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map() : root{ nullptr }, size{ 0 } {}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(const Map<TKey, TValue>& other) : size{ other.size }
{
    Stack<Node*> s;
    s.Push(root);

    while (!s.Empty())
    {
        Node* node = s.Pop();

        if (node->left) { s.Push(node->left); }
        if (node->right) { s.Push(node->right); }

        Insert(node->key, node->value);
    }

    size = 0;
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
inline void Map<TKey, TValue>::Destroy()
{
    Clear();
}

template<typename TKey, typename TValue>
inline Map<TKey, TValue>& Map<TKey, TValue>::operator=(const Map<TKey, TValue>& other)
{
    size = other.size;

    Stack<Node*> s;
    s.Push(root);

    while (!s.Empty())
    {
        Node* node = s.Pop();

        if (node->left) { s.Push(node->left); }
        if (node->right) { s.Push(node->right); }

        Insert(node->key, node->value);
    }

    size = 0;
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
    if (root)
    {
        Stack<Node*> s;
        s.Push(root);

        while (!s.Empty())
        {
            Node* node = s.Pop();

            if (node->left) { s.Push(node->left); }
            if (node->right) { s.Push(node->right); }

            delete node;
        }

        size = 0;
        root = nullptr;
    }
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

        while (node && node->key != key)
        {
            parent = node;
            node = (&node->left)[right = key > node->key];
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

            if (node->key == root->key) { root = replace; }
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
    Stack<Node*> s;
    s.Push(root);

    while (!s.Empty())
    {
        Node* node = s.Pop();

        if (node->value == value) { return true; }

        if (node->left) { s.Push(node->left); }
        if (node->right) { s.Push(node->right); }
    }

    return false;
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