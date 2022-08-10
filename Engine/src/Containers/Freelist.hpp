#pragma once

#include "Defines.hpp"

#include "Platform/Platform.hpp"

struct NH_API Freelist
{
    struct Node
    {
        Node(const U64& size, const U64& offset, Node* next = nullptr) : size{ size }, offset{ offset }, next{ next } {}
        ~Node() { next = nullptr; }

        void* operator new(U64 size) { return Platform::Allocate(sizeof(Node), false); }
        void operator delete(void* ptr) { Platform::Free(ptr, false); }

        U64 size;
        U64 offset;

        Node* next;
    };

public:
    Freelist(U64 size = 0);
    Freelist(const Freelist& other) = delete;
    Freelist(Freelist&& other) noexcept;
    ~Freelist();
    void Destroy();

    void* operator new(U64 size) { return Platform::Allocate(sizeof(Freelist), false); }
    void operator delete(void* ptr) { Platform::Free(ptr, false); }

    Freelist& operator=(const Freelist& other) = delete;
    Freelist& operator=(Freelist&& other) noexcept;

    U64 AllocateBlock(U64 size);
    bool FreeBlock(U64 size, U64 offset);
    bool Resize(U64 size);
    //TODO: Defragment

    U64 TotalSize() const { return totalSize; }
    U64 FreeSpace() const { return freeSpace; }

private:
    U64 totalSize;
    U64 freeSpace;

    Node* head;
};