#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct Stack
{
    struct Node
    {
        Node(const T& value) : value{ value }, next{ nullptr } { }
        ~Node() { next = nullptr; }

        void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
        void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

        T value;
        Node* next;
    };

public:
    NH_API Stack() : size{ 0 }, head{ nullptr } {}
    NH_API Stack(const Stack& other);
    NH_API Stack(Stack&& other);
    NH_API ~Stack();
    NH_API void Destroy();

    NH_API Stack& operator=(const Stack& other);
    NH_API Stack& operator=(Stack&& other);

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
};

template<typename T>
Stack<T>::Stack(const Stack& other)
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
Stack<T>::Stack(Stack&& other) : size{ other.size }, head{ other.head }
{
    other.size = 0;
    other.head = nullptr;
}

template<typename T>
Stack<T>::~Stack()
{
    Clear();
}

template<typename T>
void Stack<T>::Destroy()
{
    Clear();
}

template<typename T>
Stack<T>& Stack<T>::operator=(const Stack<T>& other)
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
Stack<T>& Stack<T>::operator=(Stack<T>&& other)
{
    size = other.size;
    head = other.head;

    other.size = 0;
    other.head = nullptr;
}

template<typename T>
void Stack<T>::Clear()
{
    while (head)
    {
        Pop();
    }
}


template<typename T>
inline T&& Stack<T>::Pop()
{
    if (head)
    {
        --size;
        Node* tempNode = head;
        head = head->next;
        T value = tempNode->value;
        delete tempNode;
        return Move(value);
    }

    return Move(T{});
}

template<typename T>
inline void Stack<T>::Push(const T& value)
{
    ++size;
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
}

template<typename T>
inline void Stack<T>::Push(T&& value)
{
    ++size;
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
}
