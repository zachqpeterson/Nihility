#include "Memory.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include "Containers/String.hpp"

//TODO: Remove
#include <stdio.h>

#ifdef NH_DEBUG
static String memoryTagNames[MEMORY_TAG_MAX_TAGS] = {
    "TOTAL      ",
    "UNKNOWN    ",
    "DATA_STRUCT",
    "STRING     ",
    "LINEAR_ALLC",
    "APPLICATION",
    "JOB        ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      ",
    "RESOURCE   " };
#endif

U64 Memory::totalAllocSize;
U64 Memory::allocCount;
U64 Memory::deallocCount;
U64 Memory::taggedAllocations[MEMORY_TAG_MAX_TAGS];
void* Memory::allocatorBlock;

bool Memory::Initialize(U64 memoryRequirement)
{
    //TODO: Memory alignment
    //NOTE: Don't allocate this block until we have an allocator
    //allocatorBlock = Platform::Allocate(memoryRequirement, false);

    totalAllocSize = memoryRequirement;
    allocCount = 0;
    deallocCount = 0;

    return true;
}

void Memory::Shutdown()
{
    Platform::Free(allocatorBlock, totalAllocSize);
    allocatorBlock = nullptr;
}

void* Memory::Allocate(U64 size, MemoryTag tag)
{
    taggedAllocations[tag] += size;
    taggedAllocations[MEMORY_TAG_TOTAL] += size;
    ++allocCount;

    //TODO: Custom allocator
    //TODO: Memory alignment
    void* block = Platform::Allocate(size, false);
    Platform::Zero(block, size);
    return block;
}

void Memory::Free(void* block, U64 size, MemoryTag tag)
{
    taggedAllocations[tag] -= size;
    taggedAllocations[MEMORY_TAG_TOTAL] -= size;
    ++deallocCount;

    //TODO: Custom allocator
    //TODO: Memory alignment
    Platform::Free(block, false);
}

void* Memory::Zero(void* block, U64 size)
{
    return Platform::Zero(block, size);
}

void* Memory::Copy(void* dest, const void* source, U64 size)
{
    return Platform::Copy(dest, source, size);
}

void* Memory::Set(void* dest, I32 value, U64 size)
{
    return Platform::Set(dest, value, size);
}

U8 Memory::BigEndianU8(U8* data)
{
    U32 result = 0;
    for (U8 i = 0; i < 1; ++i)
    {
        result <<= 8;
        result |= *(data + i);
    }

    return result;
}

U16 Memory::BigEndianU16(U8* data)
{
    U32 result = 0;
    for (U8 i = 0; i < 2; ++i)
    {
        result <<= 8;
        result |= *(data + i);
    }

    return result;
}

U32 Memory::BigEndianU32(U8* data)
{
    U32 result = 0;
    for (U8 i = 0; i < 4; ++i)
    {
        result <<= 8;
        result |= *(data + i);
    }

    return result;
}

U64 Memory::BigEndianU64(U8* data)
{
    U32 result = 0;
    for (U8 i = 0; i < 8; ++i)
    {
        result <<= 8;
        result |= *(data + i);
    }

    return result;
}

void Memory::GetMemoryStats()
{
#ifdef NH_DEBUG
    const U64 gib = 1024 * 1024 * 1024;
    const U64 mib = 1024 * 1024;
    const U64 kib = 1024;

    U64 allocAmounts[MEMORY_TAG_MAX_TAGS];
    Copy(allocAmounts, taggedAllocations, sizeof(U64) * MEMORY_TAG_MAX_TAGS);

    U64 currAlloc = allocCount;
    U64 currDealloc = deallocCount;

    String buffer("System memory use (tagged):\n");
    for (U32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
    {
        String unit;
        F32 amount = 1.0f;
        if (allocAmounts[i] >= gib)
        {
            unit = "GB";
            amount = allocAmounts[i] / (F32)gib;
        }
        else if (allocAmounts[i] >= mib)
        {
            unit = "MB";
            amount = allocAmounts[i] / (F32)mib;
        }
        else if (allocAmounts[i] >= kib)
        {
            unit = "KB";
            amount = allocAmounts[i] / (F32)kib;
        }
        else
        {
            unit = "B";
            amount = (F32)allocAmounts[i];
        }

        String add("{}: {}{}\n");
        add.Format(memoryTagNames[i], amount, unit);
        buffer.Append(add);
    }

    buffer.Append("allocCount: ");
    buffer.Append(String(currAlloc));
    buffer.Append("\n");
    buffer.Append("deallocCount: ");
    buffer.Append(String(currDealloc));

    Logger::Debug(buffer);
#endif
}