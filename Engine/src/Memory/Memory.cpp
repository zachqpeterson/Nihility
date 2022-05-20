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

U64 Memory::totalAllocSize;
U64 Memory::allocCount;
U64 Memory::taggedAllocations[MEMORY_TAG_MAX_TAGS];
void* Memory::allocatorBlock;

bool Memory::Initialize(U64 memoryRequirement)
{
    //TODO: Memory alignment
    //NOTE: Don't allocate this block until we have an allocator
    //allocatorBlock = Platform::Allocate(memoryRequirement, false);

    totalAllocSize = memoryRequirement;
    allocCount = 0;

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

    String buffer("System memory use (tagged):\n");
    for (U32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
    {
        String unit;
        F32 amount = 1.0f;
        if (taggedAllocations[i] >= gib)
        {
            unit = "GiB";
            amount = taggedAllocations[i] / (F32)gib;
        }
        else if (taggedAllocations[i] >= mib)
        {
            unit = "MiB";
            amount = taggedAllocations[i] / (F32)mib;
        }
        else if (taggedAllocations[i] >= kib)
        {
            unit = "KiB";
            amount = taggedAllocations[i] / (F32)kib;
        }
        else
        {
            unit = "B";
            amount = (F32)taggedAllocations[i];
        }

        String add;
        add.Format("  %s: %.2f%s\n", (const char*)memoryTagNames[i], amount, (const char*)unit);
        buffer.Append(add);
    }

    LOG_DEBUG(buffer);

#endif
}