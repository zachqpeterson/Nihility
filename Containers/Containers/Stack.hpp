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
    Stack() : size{ 0 }, head{ nullptr } {}
    Stack(const Stack& other)
    {
        Node* node = other.head;

        while (node)
        {
            PushBack(node->_value);
            node = node->_next;
        }

        return *this;
    }
    Stack(Stack&& other) : size{ other.size }, head{ other.head }
    {
        other.size = 0;
        other.head = nullptr;
    }
    ~Stack() { Clear(); }
    void Destroy() { Clear(); }

    void* operator new(U64 size) { return Memory::Allocate(sizeof(Stack), MEMORY_TAG_DATA_STRUCT); }
    void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Stack), MEMORY_TAG_DATA_STRUCT); }

    Stack& operator=(const Stack& other)
    {
        if (head) { Clear(); }

        Node* node = other.head;

        while (node)
        {
            PushBack(node->value);
            node = node->next;
        }

        return *this;
    }
    Stack& operator=(Stack&& other)
    {
        size = other.size;
        head = other.head;

        other.size = 0;
        other.head = nullptr;
    }

    void Clear()
    {
        while (head)
        {
            Pop();
        }
    }
    const bool Empty() const { return !size; }
    const U64& Size() const { return size; }

    T& Peek() { return *head; }
    T&& Pop()
    {
        if (head)
        {
            --size;
            Node* tempNode = head;
            head = head->next;
            T&& value = Move(tempNode->value);
            delete tempNode;
            return Move(value);
        }

        return Move(T{});
    }
    void Push(const T& value)
    {
        ++size;
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }
    void Push(T&& value)
    {
        ++size;
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

private:
    U64 size;
    Node* head;
};