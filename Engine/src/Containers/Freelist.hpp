/** @file Freelist.hpp */
#pragma once

#include "Defines.hpp"

class Freelist
{
public:
    NH_API Freelist(U64 size, void* block);
    NH_API void Destroy();

    NH_API bool AllocateBlock(U64 size, U64* out_offset);
    NH_API bool FreeBlock(U64 size, U64 offset);
    NH_API bool Resize(void* new_memory, U64 new_size, void** out_old_memory);
    NH_API void Clear();
    NH_API bool FreeSpace();

    static NH_API U64 GetMemoryRequirement(U64 size);

private:
    void* memory;

    struct Node* GetNode();
    void ReturnNode(struct Node* node);
};