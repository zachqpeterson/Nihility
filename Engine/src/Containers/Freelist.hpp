#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"

struct NH_API Freelist
{
    struct Node
    {
        Node(const U32& size, const U32& offset, Node* next = nullptr) : size{ size }, offset{ offset }, next{ next } {}
        ~Node() { next = nullptr; }

        void* operator new(U64 size) { return Memory::Allocate(sizeof(Node), MEMORY_TAG_DATA_STRUCT); }
        void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Node), MEMORY_TAG_DATA_STRUCT); }

        U32 size;
        U32 offset;

        Node* next;
    };

public:
    Freelist(U32 size = 0);
    Freelist(const Freelist& other) = delete;
    Freelist(Freelist&& other);
    ~Freelist();
    void Destroy();

    Freelist& operator=(const Freelist& other) = delete;
    Freelist& operator=(Freelist&& other);

    U32 AllocateBlock(U32 size);
    bool FreeBlock(U32 size, U32 offset);
    bool Resize(U32 size);
    //TODO: Defragment

    U32 FreeSpace() const { return freeSpace; }

private:
    U32 totalSize;
    U32 freeSpace;

    Node* head;
};