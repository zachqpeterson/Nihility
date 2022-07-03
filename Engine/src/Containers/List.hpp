#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

template<typename T>
struct NH_API List
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

    struct NH_API Iterator
    {
        Iterator() : ptr{ head } {}
        Iterator(Node* node) : ptr{ node } {}

        T& operator* () const { return ptr->value; }
        T* operator-> () { return &(ptr->value); }

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

        NH_API friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
        NH_API friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
        NH_API friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
        NH_API friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
        NH_API friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }
        NH_API friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }

        operator bool() { return ptr; }

    private:
        Node* ptr;
    };

public:
    List() : size{ 0 }, head{ nullptr }, tail{ nullptr } {}
    List(const List& other);
    List(List&& other);
    ~List();
    void Destroy();

    void* operator new(U64 size) { return Memory::Allocate(sizeof(List), MEMORY_TAG_DATA_STRUCT); }
    void operator delete(void* ptr) { Memory::Free(ptr, sizeof(List), MEMORY_TAG_DATA_STRUCT); }

    List& operator=(const List& other);
    List& operator=(List&& other);

    void Assign(const List& other);
    void Assign(List&& other);

    T& Front() { return head->value; }
    const T& Front() const { return head->value; }
    T& Back() { return tail->value; }
    const T& Back() const { return tail->value; }

    Iterator begin() { return Iterator(head); }
    Iterator end() { if (tail) { return Iterator(tail->next); } return Iterator(tail); }

    const bool Empty() const { return !size; }
    const U64& Size() const { return size; }

    void Clear();

    //TODO: Insert with range
    T&& RemoveAt(U64 index);
    Iterator Erase(Iterator& it);

    void PushFront(const T& value);
    void PushFront(T&& value);
    T&& PopFront();
    void PushBack(const T& value);
    void PushBack(T&& value);
    T&& PopBack();

    T&& Remove(const T& value);
    T&& Remove(Iterator& it);
    void Reverse();
    //TODO: Sorts

    const bool Contains(const T& value) const;
    const U64 Search(const T& value);
    Iterator Find(const T& value);

    T& Get(U64 index);
    const T& Get(U64 index) const;
    T& operator[](U64 i);
    const T& operator[](U64 i) const;

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
        PushBack(node->value);
        node = node->next;
    }
}

template<typename T>
inline List<T>::List(List<T>&& other) : size{ size }, head{ other.head }, tail{ other.tail }
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
inline void List<T>::Destroy()
{
    Clear();
}

template<typename T>
inline List<T>& List<T>::operator=(const List<T>& other)
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

template<typename T>
inline List<T>& List<T>::operator=(List<T>&& other)
{
    if (head) { clear(); }

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
inline void List<T>::Assign(List&& other)
{
    if (head) { clear(); }

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
    Node* newNode = new Node(value);
    if (head)
    {
        head->prev = newNode;
        newNode->next = head;
    }
    else { tail = newNode; }

    head = newNode;
}

template<typename T>
inline void List<T>::PushFront(T&& value)
{
    ++size;
    Node* newNode = new Node(value);
    if (head)
    {
        head->prev = newNode;
        newNode->next = head;
    }
    else { tail = newNode; }

    head = newNode;
}

template<typename T>
inline T&& List<T>::PopFront()
{
    if (head)
    {
        --size;
        Node* tempNode = head;
        head = head->next;
        if(head) { head->prev = nullptr; }
        T value = tempNode->value;
        delete tempNode;
        return Move(value);
    }

    return Move(T{});
}

template<typename T>
inline void List<T>::PushBack(const T& value)
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
inline void List<T>::PushBack(T&& value)
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
inline T&& List<T>::PopBack()
{
    if (tail)
    {
        --size;
        Node* tempNode = tail;
        tail = tail->prev;
        tail->next = nullptr;
        T value = tempNode->value;
        delete tempNode;
        return Move(value);
    }

    return Move(T{});
}

template<typename T>
inline T&& List<T>::Remove(const T& value)
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

            T value = node->value;
            delete node;
            return Move(value);
        }

        node = node->next;
    }
}

template<typename T>
inline T&& List<T>::Remove(List<T>::Iterator& it)
{
    --size;
    Node* node = it.ptr;
    Node* nextNode = node->next;
    Node* prevNode = node->prev;

    if (prevNode) { prevNode->next = nextNode; }
    else { head = nextNode; }

    if (nextNode) { nextNode->prev = prevNode; }
    else { tail = prevNode; }

    T value = node->value;
    delete node;
    return Move(value);
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
    for (U64 i = 0; i < index; ++i)
    {
        node = node->next;
    }

    node->prev = node->next;
    node->next = node->prev;

    T value = node->value;

    delete node;

    return Move(value);
}

template<typename T>
inline typename List<T>::Iterator List<T>::Erase(List<T>::Iterator& it)
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

    delete node;

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