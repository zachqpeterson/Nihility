/** @file Freelist.hpp */
#pragma once

#include "Defines.hpp"

struct Freelist
{
    struct Node
    {
        U64 offset;
        U64 size;
        struct Node* next;
    };

public:
    NH_API Freelist(U64 size = 0);
    NH_API void Destroy();

    NH_API bool AllocateBlock(U64 size, U64* outOffset);
    NH_API bool FreeBlock(U64 size, U64 offset);
    NH_API bool Resize(U64 newSize);
    NH_API void Cleanup();
    NH_API bool FreeSpace();

private:
    U64 totalSize;
    U64 maxEntries;
    Node* head;
    Node* nodes;

    struct Node* GetNode();
    void ReturnNode(struct Node* node);
};