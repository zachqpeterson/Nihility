#include "Memory.hpp"

#include "Platform/Platform.hpp"

//TODO: custom string class
//static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
//    "UNKNOWN    ",
//    "ARRAY      ",
//    "LINEAR_ALLC",
//    "DARRAY     ",
//    "DICT       ",
//    "RING_QUEUE ",
//    "BST        ",
//    "STRING     ",
//    "APPLICATION",
//    "JOB        ",
//    "TEXTURE    ",
//    "MAT_INST   ",
//    "RENDERER   ",
//    "GAME       ",
//    "TRANSFORM  ",
//    "ENTITY     ",
//    "ENTITY_NODE",
//    "SCENE      ",
//    "RESOURCE   " };

U64 Memory::totalAllocSize;
U64 Memory::totalAllocated;
U64 Memory::allocCount;
U64 Memory::taggedAllocations[MEMORY_TAG_MAX_TAGS];
void* Memory::allocatorBlock;

bool Memory::Initialize(U64 memoryRequirement)
{
    totalAllocSize = memoryRequirement;
    totalAllocated = 0;
    allocCount = 0;

    //TODO: Memory alignment
    allocatorBlock = Platform::Allocate(memoryRequirement, false);

    return true;
}

void Memory::Shutdown()
{
    Platform::Free(allocatorBlock, totalAllocSize);
    allocatorBlock = nullptr;
}

void* Memory::Allocate(U64 size, MemoryTag tag)
{
    totalAllocated += size;
    taggedAllocations[tag] += size;
    ++allocCount;

    //TODO: Custom allocator
    //TODO: Memory alignment
    void* block = Platform::Allocate(size, false);
    Platform::ZeroMemory(block, size);
    return block;
}

void Memory::Free(void* block, U64 size, MemoryTag tag)
{
    totalAllocated -= size;
    taggedAllocations[tag] -= size;
    //TODO: Custom allocator
    //TODO: Memory alignment
    Platform::Free(block, false);
}

void* Memory::ZeroMemory(void* block, U64 size)
{
    return Platform::ZeroMemory(block, size);
}

void* Memory::CopyMemory(void* dest, const void* source, U64 size)
{
    return Platform::CopyMemory(dest, source, size);
}

void* Memory::SetMemory(void* dest, I32 value, U64 size)
{
    return Platform::SetMemory(dest, value, size);
}

void GetMemoryStats()
{
#ifdef NH_DEBUG
    //TODO: Print memory stats
    
#endif
}