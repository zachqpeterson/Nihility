#include "DynamicAllocator.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"

DynamicAllocator::DynamicAllocator(U64 size)
{
    if (size)
    {
        memory = Platform::Allocate(size, false);
        allocations.Resize(size);
    }
}

DynamicAllocator::~DynamicAllocator()
{
    Destroy();
}

void DynamicAllocator::Destroy()
{
    if (memory)
    {
        allocations.Destroy();
        Platform::Free(memory, false);
    }
}

void* DynamicAllocator::operator new(U64 size)
{
    return Platform::Allocate(sizeof(DynamicAllocator), false);
}

void DynamicAllocator::operator delete(void* ptr)
{
    Platform::Free(ptr, false);
}

DynamicAllocator& DynamicAllocator::operator=(DynamicAllocator&& other) noexcept
{
    memory = other.memory;
    allocations = Move(other.allocations);
    other.memory = nullptr;

    return *this;
}

void* DynamicAllocator::Allocate(U64 size)
{
    return (U8*)memory + allocations.AllocateBlock(size);
}

bool DynamicAllocator::Free(void* block, U64 size)
{
    U64 offset = (U8*)block - (U8*)memory;
    if (!allocations.FreeBlock(size, offset)) { return false; }
    block = nullptr;
    return true;
}