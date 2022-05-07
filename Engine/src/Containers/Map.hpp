#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

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
    Map();
    Map(const Map& other);
    Map(Map&& other) noexcept;

    Map& operator=(const Map& other);
    Map& operator=(Map&& other) noexcept;

    void Insert(const TKey& key, const TValue& value);
    void Insert(TKey&& key, TValue&& value);

    bool Find(const TValue& value);
    bool Find(const TKey& key);

    TValue& operator[](const TKey& key);
    const TValue& operator[](const TKey& key) const;

private:
    Node* root;
};
