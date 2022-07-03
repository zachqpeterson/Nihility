#include "Memory.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include "Containers/String.hpp"
#include "DynamicAllocator.hpp"

U64 Memory::totalAllocSize;
U64 Memory::allocCount;
U64 Memory::deallocCount;
U64 Memory::taggedAllocations[MEMORY_TAG_MAX_TAGS];

DynamicAllocator* Memory::allocator;

bool Memory::Initialize(U64 memoryRequirement)
{
    allocator = new DynamicAllocator(memoryRequirement);

    totalAllocSize = memoryRequirement;
    allocCount = 0;
    deallocCount = 0;

    return true;
}

void Memory::Shutdown()
{
    allocator->Destroy();
}

void* Memory::Allocate(U64 size, MemoryTag tag)
{
    taggedAllocations[tag] += size;
    taggedAllocations[MEMORY_TAG_TOTAL] += size;
    ++allocCount;

    //TODO: Memory alignment
    void* block = allocator->Allocate(size);
    return Platform::Zero(block, size);
}

void Memory::Free(void* block, U64 size, MemoryTag tag)
{
    taggedAllocations[tag] -= size;
    taggedAllocations[MEMORY_TAG_TOTAL] -= size;
    ++deallocCount;

    //TODO: Memory alignment
    allocator->Free(block, size);
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

//Byte stuff
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

U32 Memory::HighBit(U32 z)
{
    U32 n = 0;
    if (z == 0) { return 0; }
    if (z >= 0x10000) { n += 16; z >>= 16; }
    if (z >= 0x00100) { n += 8; z >>= 8; }
    if (z >= 0x00010) { n += 4; z >>= 4; }
    if (z >= 0x00004) { n += 2; z >>= 2; }
    if (z >= 0x00002) { n += 1; }
    return n;
}

U32 Memory::BitCount(U32 a)
{
    a = (a & 0x55555555) + ((a >> 1) & 0x55555555);
    a = (a & 0x33333333) + ((a >> 2) & 0x33333333);
    a = (a + (a >> 4)) & 0x0f0f0f0f;
    a = (a + (a >> 8));
    a = (a + (a >> 16));
    return a & 0xff;
}

U32 Memory::ShiftSigned(U32 v, I32 shift, I32 bits)
{
    static U32 mulTable[9] = {
       0,
       0xff, 0x55, 0x49, 0x11,
       0x21, 0x41, 0x81, 0x01,
    };

    static U32 shiftTable[9] = {
       0, 0,0,1,0,2,4,6,0,
    };

    if (shift < 0) { v <<= -shift; }
    else { v >>= shift; }
    ASSERT_DEBUG(v < 256);
    v >>= (8 - bits);
    ASSERT_DEBUG(bits >= 0 && bits <= 8);

    return (v * mulTable[bits]) >> shiftTable[bits];
}

void Memory::GetMemoryStats()
{
#ifdef NH_DEBUG
    static const char* memoryTagNames[MEMORY_TAG_MAX_TAGS] = {
        "TOTAL      ",
        "UNKNOWN    ",
        "DATA_STRUCT",
        "STRING     ",
        "APPLICATION",
        "JOB        ",
        "MAT_INST   ",
        "RENDERER   ",
        "GAME       ",
        "TRANSFORM  ",
        "ENTITY     ",
        "RESOURCE   " };

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