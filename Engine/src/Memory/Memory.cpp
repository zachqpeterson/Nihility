#include "Memory.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include "Containers/String.hpp"

//TODO: Remove
#include <stdio.h>

#ifdef NH_DEBUG
static const char* memoryTagNames[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "DATA_STRUCT",
    "LINEAR_ALLC",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      ",
    "RESOURCE   " };
#endif

struct MemoryState
{
    U64 totalAllocSize; //total space we can allocate
    U64 totalAllocated; //how much space we've allocated
    U64 allocCount; //how many allocations made
    U64 taggedAllocations[MEMORY_TAG_MAX_TAGS]; //how much space we've allocated for each tag
    void* allocatorBlock; //pointer to the block of memory we have access to
};

static MemoryState* memoryState;

bool Memory::Initialize(U64 memoryRequirement)
{
    //TODO: Somehow queue message "Initializing the memory system with a size of %d bytes" to logger before initialization

    //TODO: Memory alignment
    void* block = Platform::Allocate(memoryRequirement, false);

    memoryState = (MemoryState*)block;

    memoryState->totalAllocSize = memoryRequirement;
    memoryState->totalAllocated = sizeof(MemoryState);
    memoryState->allocCount = 1;
    memoryState->allocatorBlock = (void*)block;
    memoryState->taggedAllocations[MEMORY_TAG_APPLICATION] = sizeof(MemoryState);

    return true;
}

void Memory::Shutdown()
{
    Platform::Free(memoryState->allocatorBlock, memoryState->totalAllocSize);
    memoryState = nullptr;
}

void* Memory::Allocate(U64 size, MemoryTag tag)
{
    memoryState->totalAllocated += size;
    memoryState->taggedAllocations[tag] += size;
    ++memoryState->allocCount;

    //TODO: Custom allocator
    //TODO: Memory alignment
    void* block = Platform::Allocate(size, false);
    Platform::ZeroMemory(block, size);
    return block;
}

void Memory::Free(void* block, U64 size, MemoryTag tag)
{
    memoryState->totalAllocated -= size;
    memoryState->taggedAllocations[tag] -= size;
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

void Memory::GetMemoryStats()
{
#ifdef NH_DEBUG
    const U64 gib = 1024 * 1024 * 1024;
    const U64 mib = 1024 * 1024;
    const U64 kib = 1024;

    String buffer("System memory use (tagged):\n");
    for (U32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
    {
        String unit;
        F32 amount = 1.0f;
        if (memoryState->taggedAllocations[i] >= gib)
        {
            unit = "GiB";
            amount = memoryState->taggedAllocations[i] / (F32)gib;
        }
        else if (memoryState->taggedAllocations[i] >= mib)
        {
            unit = "MiB";
            amount = memoryState->taggedAllocations[i] / (F32)mib;
        }
        else if (memoryState->taggedAllocations[i] >= kib)
        {
            unit = "KiB";
            amount = memoryState->taggedAllocations[i] / (F32)kib;
        }
        else
        {
            unit = "B";
            amount = (float)memoryState->taggedAllocations[i];
        }

        String add(new char[100]);
        add.Format("  %s: %.2f%s\n", (char*)memoryTagNames[i], amount, (char*)unit);
        //TODO: The crash here has to do with the amount of times we call this
        buffer.Append(add);
    }

    LOG_DEBUG(buffer);

#endif
}