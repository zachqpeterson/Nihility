#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct List
{
    struct Node
    {
        Node(const T& value) : value{ value }, next{ nullptr }, prev{ nullptr } {}

        T value;

        Node* next;
        Node* prev;
    };

    struct Iterator
    {
        Iterator() : ptr{ head } {}
        Iterator(Node* node) : ptr{ node } {}

        NH_API T& operator* () const { return ptr->value; }
        NH_API T* operator-> () { return &(ptr->value); }

        NH_API Iterator& operator++ () { ptr = ptr->next; return *this; }
        NH_API Iterator& operator-- () { ptr = ptr->prev; return *this; }

        NH_API Iterator operator++ (int)
        {
            Iterator temp = *this;
            ptr = ptr->next;
            return temp;
        }

        NH_API Iterator operator-- (int)
        {
            Iterator temp = *this;
            ptr = ptr->prev;
            return temp;
        }

        NH_API Iterator operator+= (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->next; }
            }

            return *this;
        }

        NH_API Iterator operator-= (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->prev; }
            }

            return *this;
        }

        NH_API Iterator operator+ (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->next; }
            }

            return *this;
        }

        NH_API Iterator operator- (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->prev; }
            }

            return *this;
        }

        NH_API friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
        NH_API friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
        NH_API friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
        NH_API friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
        NH_API friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }
        NH_API friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }

        NH_API operator bool() { return ptr; }

    private:
        Node* ptr;
    };

public:
    NH_API List() : size{ 0 }, head{ nullptr }, tail{ nullptr } {}
    NH_API List(const List& other);
    NH_API List(List&& other) noexcept;
    NH_API ~List();
    
    NH_API List& operator=(const List& other);
    NH_API List& operator=(List&& other) noexcept;
    
    NH_API void Assign(const List& other);
    NH_API void Assign(List&& other) noexcept;
    
    NH_API T& Front() { return head->value; }
    NH_API const T& Front() const { return head->value; }
    NH_API T& Back() { return tail->value; }
    NH_API const T& Back() const { return tail->value; }
    
    NH_API Iterator begin() { return Iterator(head); }
    NH_API Iterator end() { if (tail) { return Iterator(tail->next); } return Iterator(tail); }
    
    NH_API const bool Empty() const { return !size; }
    NH_API const U64& Size() const { return size; }
    
    NH_API void Clear();
    
    //TODO: Insert with range
    NH_API T&& RemoveAt(U64 index);
    NH_API Iterator Erase(Iterator it);
    
    NH_API void PushFront(const T& value);
    NH_API void PushFront(T&& value) noexcept;
    NH_API T&& PopFront() noexcept;
    NH_API void PushBack(const T& value);
    NH_API void PushBack(T&& value) noexcept;
    NH_API T&& PopBack() noexcept;
    
    NH_API void Remove(const T& value);
    NH_API void Reverse();
    //TODO: Sorts
    
    NH_API const bool Contains(const T& value) const;
    NH_API const U64 Search(const T& value);
    NH_API Iterator Find(const T& value);
    
    NH_API T& Get(U64 index);
    NH_API const T& Get(U64 index) const;
    NH_API T& operator[](U64 i);
    NH_API const T& operator[](U64 i) const;

private:
    U64 size;
    Node* head;
    Node* tail;
};

template<typename T>
inline List<T>::List(const List<T>& other)
{
    Node* node = other.head;

    while (node)
    {
        PushBack(node->_value);
        node = node->_next;
    }
}

template<typename T>
inline List<T>::List(List<T>&& other) noexcept : size{ size }, head{ other.head }, tail{ other.tail }
{
    other.size = 0;
    other.head = nullptr;
    other.tail = nullptr;
}

template<typename T>
inline List<T>::~List()
{
    Clear();
}

template<typename T>
inline List<T>& List<T>::operator=(const List<T>& other)
{
    Node* node = other.head;

    while (node)
    {
        PushBack(node->_value);
        node = node->_next;
    }

    return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(List<T>&& other) noexcept
{
    size = other.size;
    head = other.head;
    tail = other.tail;

    other.size = 0;
    other.head = nullptr;
    other.tail = nullptr;

    return *this;
}

template<typename T>
inline void List<T>::Assign(const List& other)
{
    Node* node = other.head;

    while (node)
    {
        PushBack(node->_value);
        node = node->_next;
    }

    return *this;
}

template<typename T>
inline void List<T>::Assign(List&& other) noexcept
{
    size = other.size;
    head = other.head;
    tail = other.tail;

    other.size = 0;
    other.head = nullptr;
    other.tail = nullptr;

    return *this;
}

template<typename T>
inline void List<T>::PushFront(const T& value)
{
    ++size;
    Node* newNode = Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = value;
    newNode->prev = nullptr;
    newNode->next = nullptr;
    if (head)
    {
        head->prev = newNode;
        newNode->next = head;
    }
    else { tail = newNode; }

    head = newNode;
}

template<typename T>
inline void List<T>::PushFront(T&& value) noexcept
{
    ++size;
    Node* newNode = Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = Move(value);
    newNode->prev = nullptr;
    newNode->next = nullptr;
    if (head)
    {
        head->prev = newNode;
        newNode->next = head;
    }
    else { tail = newNode; }

    head = newNode;
}

template<typename T>
inline T&& List<T>::PopFront() noexcept
{
    if (head)
    {
        --size;
        Node* tempNode = head;
        head = head->next;
        head->prev = nullptr;
        T value = tempNode->value;
        Memory::Free(tempNode, sizeof(Node), MEMORY_TAG_DATA_STRUCT);
        return move(value);
    }
}

template<typename T>
inline void List<T>::PushBack(const T& value)
{
    Node* newNode = (Node*)Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = value;
    newNode->prev = nullptr;
    newNode->next = nullptr;
    ++size;
    if (head)
    {
        tail->next = newNode;
        newNode->prev = tail;
    }
    else { head = newNode; }

    tail = newNode;
}

template<typename T>
inline void List<T>::PushBack(T&& value) noexcept
{
    Node* newNode = (Node*)MemoryMemory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = Move(value);
    newNode->prev = nullptr;
    newNode->next = nullptr;
    ++size;
    if (head)
    {
        tail->next = newNode;
        newNode->prev = tail;
    }
    else { head = newNode; }

    tail = newNode;
}

template<typename T>
inline T&& List<T>::PopBack() noexcept
{
    if (tail)
    {
        --size;
        Node* tempNode = tail;
        tail = tail->prev;
        tail->next = nullptr;
        T value = tempNode->value;
        Memory::Free(tempNode, sizeof(Node), MEMORY_TAG_DATA_STRUCT);
        return move(value);
    }
}

template<typename T>
inline void List<T>::Remove(const T& value)
{
    Node* node = head;
    while (node)
    {
        if (node->value == value)
        {
            --size;
            Node* nextNode = node->next;
            Node* prevNode = node->prev;

            if (prevNode) { prevNode->next = nextNode; }
            else { head = nextNode; }

            if (nextNode) { nextNode->prev = prevNode; }
            else { tail = prevNode; }

            Memory::Free(node, sizeof(Node), MEMORY_TAG_DATA_STRUCT);
            node = nextNode;
            continue;
        }

        node = node->next;
    }
}

template<typename T>
inline void List<T>::Reverse()
{
    Node* node = head;
    Node* prev = nullptr;

    while (node)
    {
        Node* next = node->next;
        node->next = node->prev;
        prev = node;
        node = next;
    }

    head = prev;
}

template<typename T>
inline void List<T>::Clear()
{
    for (U64 i = 0; i < Size(); ++i)
    {
        PopFront();
    }
}

template<typename T>
inline const bool List<T>::Contains(const T& value) const
{
    Node* node = head;
    while (node)
    {
        if (node->value == value) { return true; }

        node = node->next;
    }

    return false;
}

template<typename T>
inline T&& List<T>::RemoveAt(U64 index)
{
    ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

    --size;
    Node* node = head;
    for(U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    node->prev = node->next;
    node->next = node->prev;

    T value = node->value;

    Memory::Free(node, sizeof(Node), MEMORY_TAG_DATA_STRUCT);

    return move(value);
}

template<typename T>
inline typename List<T>::Iterator List<T>::Erase(List<T>::Iterator it)
{
    --size;
    Node* node = it.ptr;
    Node* nextNode = node->next;
    Node* prevNode = node->prev;

    if (prevNode) { prevNode->next = nextNode; }
    else { head = nextNode; }

    if (nextNode) { nextNode->prev = prevNode; }
    else { tail = prevNode; }

    List<T>::Iterator newIt = ++it;

    Memory::Free(node, sizeof(Node), MEMORY_TAG_DATA_STRUCT);

    return newIt;
}

template<typename T>
inline const U64 List<T>::Search(const T& value)
{
    Node* node = head;
    U64 index = 0;
    while (node)
    {
        if (node->value == value) { return index; }

        node = node->next;
        ++index;
    }

    return -1;
}

template<typename T>
inline typename List<T>::Iterator List<T>::Find(const T& value)
{
    Node* node = head;
    while (node)
    {
        if (node->value == value) { return Iterator(node); }

        node = node->next;
    }

    return end();
}

template<typename T>
inline T& List<T>::Get(U64 index)
{
    ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

    Node* node = head;
    for (U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    return node->value;
}

template<typename T>
inline const T& List<T>::Get(U64 index) const
{
    ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

    Node* node = head;
    for (U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    return node->value;
}


template<typename T>
inline T& List<T>::operator[](U64 i)
{
    ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

    Node* node = head;
    for (U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    return node->value;
}

template<typename T>
inline const T& List<T>::operator[](U64 i) const
{
    ASSERT_DEBUG_MSG(index < size, "Index must be less than the size of the list!");

    Node* node = head;
    for (U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    return node->value;
}