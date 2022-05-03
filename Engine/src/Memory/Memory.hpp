/** @file Defines.hpp */
#pragma once

#include "Defines.hpp"

#define Gigabytes(amount) amount * 1024 * 1024 * 1024
#define Megabytes(amount) amount * 1024 * 1024
#define Kilobytes(amount) amount * 1024

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
    static NH_API void Free(void* block, U64 size, MemoryTag tag);
    static NH_API void* ZeroMemory(void* block, U64 size);
    static NH_API void* CopyMemory(void* dest, const void* source, U64 size);
    static NH_API void* SetMemory(void* dest, I32 value, U64 size);

    //NOTE: Debug only
    static void GetMemoryStats();

private:
    static U64 totalAllocSize; //total space we can allocate
    static U64 totalAllocated; //how much space we've allocated
    static U64 allocCount; //how many allocations made
    static U64 taggedAllocations[MEMORY_TAG_MAX_TAGS]; //how much space we've allocated for each tag
    static void* allocatorBlock;
};