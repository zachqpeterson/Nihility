/** @file Defines.hpp */
#pragma once

#include "Defines.hpp"

#define Gigabytes(amount) amount * 1024ull * 1024ull * 1024ull
#define Megabytes(amount) amount * 1024ull * 1024ull
#define Kilobytes(amount) amount * 1024ull

/** @brief Tags to indicate the usage of memory allocations made in this system. */
enum MemoryTag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_DATA_STRUCT,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_RESOURCE,

    MEMORY_TAG_MAX_TAGS
};

class Memory
{
public:
    static bool Initialize(U64 memoryRequirement);
    static void Shutdown();

    static NH_API void* Allocate(U64 size, MemoryTag tag);
    static NH_API void  Free(void* block, U64 size, MemoryTag tag);
    static NH_API void* Zero(void* block, U64 size);
    static NH_API void* Copy(void* dest, const void* source, U64 size);
    static NH_API void* Set(void* dest, I32 value, U64 size);

    static NH_API U8  BigEndianU8 (U8* data);
    static NH_API U16 BigEndianU16(U8* data);
    static NH_API U32 BigEndianU32(U8* data);
    static NH_API U64 BigEndianU64(U8* data);

    //NOTE: Debug only
    static void GetMemoryStats();

private:
    Memory() = delete;

    static U64 totalAllocSize;
    static U64 totalAllocated;
    static U64 allocCount;
    static U64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
    static void* allocatorBlock;
};