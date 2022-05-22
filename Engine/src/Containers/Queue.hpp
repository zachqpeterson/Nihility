#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct Queue
{
    struct Node
    {
        Node(const T& value) : value{ value }, next{ nullptr }, prev{ nullptr } { }
        ~Node() { next = nullptr; prev = nullptr; }

        void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
        void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

        T value;
        Node* next;
        Node* prev;
    };

public:
    NH_API Queue() : size{ 0 }, head{ nullptr }, tail{ nullptr } {}
    NH_API Queue(const Queue& other);
    NH_API Queue(Queue&& other);
    NH_API ~Queue();
    NH_API void Destroy();

    NH_API Queue& operator=(const Queue& other);
    NH_API Queue& operator=(Queue&& other);

    NH_API void Clear();
    NH_API const bool Empty() const { return !size; }
    NH_API const U64& Size() const { return size; }

    NH_API T& Peek() { return *head; }
    NH_API T&& Pop();
    NH_API void Push(const T& value);
    NH_API void Push(T&& value);

private:
    U64 size;
    Node* head;
    Node* tail;
};

template<typename T>
Queue<T>::Queue(const Queue& other)
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
Queue<T>::Queue(Queue&& other) : size{ other.size }, head{ other.head }, tail{ other.tail }
{
    other.size = 0;
    other.head = nullptr;
    other.tail = nullptr;
}

template<typename T>
Queue<T>::~Queue()
{
    Clear();
}

template<typename T>
void Queue<T>::Destroy()
{
    Clear();
}

template<typename T>
Queue<T>& Queue<T>::operator=(const Queue<T>& other)
{
    if (head) { clear(); }

    Node* node = other.head;

    while (node)
    {
        PushBack(node->_value);
        node = node->_next;
    }

    return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator=(Queue<T>&& other)
{
    size = other.size;
    head = other.head;
    tail = other.tail;

    other.size = 0;
    other.head = nullptr;
    other.tail = nullptr;
}

template<typename T>
void Queue<T>::Clear()
{
    while(head)
    {
        Pop();
    }
}


template<typename T>
inline T&& Queue<T>::Pop()
{
    if (head)
    {
        --size;
        Node* tempNode = head;
        head = head->next;
        head->prev = nullptr;
        T value = tempNode->value;
        delete tempNode;
        return Move(value);
    }

    return Move(T{});
}

template<typename T>
inline void Queue<T>::Push(const T& value)
{
    Node* newNode = new Node(value);
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
inline void Queue<T>::Push(T&& value)
{
    Node* newNode = new Node(value);
    ++size;
    if (head)
    {
        tail->next = newNode;
        newNode->prev = tail;
    }
    else { head = newNode; }

    tail = newNode;
}
