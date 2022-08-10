#pragma once

#include "Defines.hpp"

#include "Containers/Freelist.hpp"

class NH_API DynamicAllocator
{
public:
    DynamicAllocator(U64 size);
    ~DynamicAllocator();
    void Destroy();

    void* operator new(U64 size);
    void operator delete(void* ptr);

    DynamicAllocator& operator=(const DynamicAllocator&) = delete;
    DynamicAllocator& operator=(DynamicAllocator&& other) noexcept;

    void* Allocate(U64 size);
    bool Free(void* block, U64 size);

private:
    Freelist allocations;
    Freelist smallAllocations;
    void* memory;
};