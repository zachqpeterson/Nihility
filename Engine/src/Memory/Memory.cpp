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

struct MemoryState
{
    U64 totalAllocSize; //total space we can allocate
    U64 totalAllocated; //how much space we've allocated
    U64 allocCount; //how many allocations made
    U64 taggedAllocations[MEMORY_TAG_MAX_TAGS]; //how much space we've allocated for each tag
    void* allocatorBlock;
};

static MemoryState* memoryState;

bool Memory::Initialize(U64 memoryRequirement)
{
    // TODO: memory alignment
    void* block = Platform::Allocate(sizeof(MemoryState) + memoryRequirement, false);
    if (!block) {
        //TODO: logger
        //KFATAL("Memory system allocation failed and the system cannot continue.");
        return false;
    }

    memoryState = (MemoryState*)block;
    memoryState->totalAllocSize = memoryRequirement;
    memoryState->totalAllocated = 0;
    memoryState->allocCount = 0;
    memoryState->allocatorBlock = (void*)((MemoryState*)block + sizeof(MemoryState));

    //TODO: logger
    //KDEBUG("Memory system successfully allocated %llu bytes.", config.total_alloc_size);
    return true;
}

void Memory::Shutdown()
{
    Platform::Free(memoryState, memoryState->totalAllocSize + sizeof(MemoryState));
    memoryState = nullptr;
}

void* Memory::Allocate(U64 size, MemoryTag tag)
{
    void* block = nullptr;
    memoryState->totalAllocated += size;
    memoryState->taggedAllocations[tag] += size;
    ++memoryState->allocCount;

    block = memoryState->allocator.Allocate(size);

    if (block) {
        Platform::ZeroMemory(block, size);
        return block;
    }

    //TODO: logger
    //KFATAL("kallocate failed to allocate successfully.");
    return nullptr;
}

void Memory::Free(void* block, U64 size, MemoryTag tag)
{
    memoryState->totalAllocated -= size;
    memoryState->taggedAllocations[tag] -= size;

    if (!memoryState->allocator.Free(block, size)) {
        // TODO: Memory alignment
        Platform::Free(block, false);
    }
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