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
        Iterator(node_t* node) : ptr{ node } {}

        T& operator* () const { return ptr->_value; }
        T* operator-> () { return &(ptr->_value); }

        Iterator& operator++ () { ptr = ptr->next; return *this; }
        Iterator& operator-- () { ptr = ptr->prev; return *this; }

        Iterator operator++ (int)
        {
            Iterator temp = *this;
            ptr = ptr->next;
            return temp;
        }

        Iterator operator-- (int)
        {
            Iterator temp = *this;
            ptr = ptr->prev;
            return temp;
        }

        Iterator operator+= (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->next; }
            }

            return *this;
        }

        Iterator operator-= (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->prev; }
            }

            return *this;
        }

        Iterator operator+ (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->next; }
            }

            return *this;
        }

        Iterator operator- (int i)
        {
            for (int j = 0; j < i; ++j)
            {
                if (ptr) { ptr = ptr->prev; }
            }

            return *this;
        }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
        friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
        friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
        friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }
        friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }

        operator bool() { return ptr; }

    private:
        Node* ptr;
    };

public:
    List() : size{ 0 }, head{ nullptr }, tail{ nullptr } {}
    List(const List& other);
    List(List&& other) noexcept;
    ~List();

    List& operator=(List&& other) noexcept;
    List& operator=(const List& other);

    void PushFront(const T& value);
    void PushFront(T&& value) noexcept;
    T&& PopFront() noexcept;
    void PushBack(const T& value);
    void PushBack(T&& value) noexcept;
    T&& PopBack() noexcept;

    T& Front() { return head->value; }
    const T& Front() const { return head->value; }
    T& Back() { return tail->value; }
    const T& Back() const { return tail->value; }

    void Remove(const T& value);
    void Reverse();
    void Clear();

    const bool Empty() const { return !size; }
    const bool Contains(const T& value) const;

    const U64& Size() const { return size; }

    Iterator Erase(Iterator it);

    Iterator begin() { return Iterator(head); }
    Iterator end() { if (tail) { return Iterator(tail->next); } return Iterator(tail); }

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
inline void List<T>::PushFront(const T& value)
{
    ++size;
    Node* newNode = Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = value;
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
    Node* newNode = Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = value;
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
    Node* newNode = Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT);
    newNode->value = Move(value);
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
inline List<T>::Iterator List<T>::Erase(List<T>::Iterator it)
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