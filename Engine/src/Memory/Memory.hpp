#pragma once

#include "Defines.hpp"

#define Gigabyte 1073741824ull
#define Megabyte 1048576ull
#define Kilobyte 1024ull

#define Gigabytes(amount) amount * Gigabyte
#define Megabytes(amount) amount * Megabyte
#define Kilobytes(amount) amount * Kilobyte

enum MemoryTag {
    MEMORY_TAG_TOTAL,
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_DATA_STRUCT,
    MEMORY_TAG_STRING,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_RESOURCE,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_AUDIO,
    MEMORY_TAG_PHYSICS,
    MEMORY_TAG_GAMEOBJECT,
    MEMORY_TAG_UI,
    MEMORY_TAG_GAME,

    MEMORY_TAG_MAX
};

class NH_API Memory
{
public:

    static void* Allocate(U64 size, MemoryTag tag);
    static void  Free(void* block, U64 size, MemoryTag tag);
    static void* LinearAllocate(U64 size);
    static void* Zero(void* block, U64 size);
    static void* Copy(void* dest, const void* source, U64 size);
    static void* Set(void* dest, I32 value, U64 size);

    //Byte stuff
    static U16 BigEndianU16(U8* data);
    static U32 BigEndianU32(U8* data);
    static U64 BigEndianU64(U8* data);
    static U32 HighBit(U32 z);
    static U32 BitCount(U32 a);
    static U32 ShiftSigned(U32 v, I32 shift, I32 bits);

    //NOTE: Debug only
    static void GetMemoryStats();

private:
    static bool Initialize(U64 memoryRequirement);
    static void Shutdown();

    static U64 totalAllocSize;
    static U64 taggedAllocations[];
    static U64 taggedAllocCounts[];
    static U64 taggedDeallocCounts[];

    static class DynamicAllocator* allocator;

    Memory() = delete;

    friend class Engine;
};