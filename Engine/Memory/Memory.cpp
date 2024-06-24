module;

#include "Defines.hpp"

#include <corecrt_malloc.h>
#include <vcruntime_string.h>

module Memory:Allocator;

import :Utilities;
import ThreadSafety;

U32 Memory::allocations = 0;
U8* Memory::memory = nullptr;
U64 Memory::totalSize = 0;

U8* Memory::dynamicPointer = nullptr;
U8* Memory::staticPointer = nullptr;

Memory::Region1kb* Memory::pool1kbPointer = nullptr;
AllocTracker Memory::free1kbAllocs{};

Memory::Region16kb* Memory::pool16kbPointer = nullptr;
AllocTracker Memory::free16kbAllocs{};

Memory::Region256kb* Memory::pool256kbPointer = nullptr;
AllocTracker Memory::free256kbAllocs{};

Memory::Region4mb* Memory::pool4mbPointer = nullptr;
AllocTracker Memory::free4mbAllocs{};

bool Memory::initialized = false;

bool Memory::Initialize()
{
	if (!SafeCheckAndSet(&initialized, 0L))
	{
		U64 maxKilobytes = DynamicMemorySize / 1024;

		U32 region4mbCount = U32(maxKilobytes / 81920);
		U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
		U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
		U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region4mbCount * 4096));

		U32 freeListMemory = (region4mbCount + region256kbCount + region16kbCount + region1kbCount) * sizeof(U32);

		totalSize = DynamicMemorySize + StaticMemorySize;

		memory = (U8*)calloc(1, totalSize + freeListMemory);

		if (!memory) { return false; }

		staticPointer = memory;
		dynamicPointer = memory + StaticMemorySize;

		pool1kbPointer = (Region1kb*)(dynamicPointer);
		pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
		pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
		pool4mbPointer = (Region4mb*)(pool256kbPointer + region256kbCount);

		U32* freeLists = (U32*)(memory + totalSize);

		free1kbAllocs.capacity = region1kbCount;
		free1kbAllocs.freeIndices = freeLists;

		free16kbAllocs.capacity = region16kbCount;
		free16kbAllocs.freeIndices = free1kbAllocs.freeIndices + free1kbAllocs.capacity;

		free256kbAllocs.capacity = region256kbCount;
		free256kbAllocs.freeIndices = free16kbAllocs.freeIndices + free16kbAllocs.capacity;

		free4mbAllocs.capacity = region4mbCount;
		free4mbAllocs.freeIndices = free256kbAllocs.freeIndices + free256kbAllocs.capacity;
	}

	return true;
}

void Memory::Shutdown()
{
	initialized = false;
}

Region Memory::GetRegion(void* pointer)
{
	static const void* upperBound = memory + totalSize;

	if (pointer >= upperBound) { return REGION_NONE; }
	else if (pointer >= pool4mbPointer) { return REGION_4MB; }
	else if (pointer >= pool256kbPointer) { return REGION_256KB; }
	else if (pointer >= pool16kbPointer) { return REGION_16KB; }
	else if (pointer >= pool1kbPointer) { return REGION_1KB; }

	return REGION_NONE;
}

Region Memory::GetRegion(U64 size)
{
	if (size <= REGION_1KB) { return REGION_1KB; }
	else if (size <= REGION_16KB) { return REGION_16KB; }
	else if (size <= REGION_256KB) { return REGION_256KB; }
	else if (size <= REGION_4MB) { return REGION_4MB; }

	return REGION_NONE;
}

void Memory::Allocate1kb(void** pointer, U64 size)
{
	if (free1kbAllocs.Full()) { Allocate16kb(pointer, size); return; }

	++allocations;
	*pointer = pool1kbPointer + free1kbAllocs.GetFree();
}

void Memory::Allocate16kb(void** pointer, U64 size)
{
	if (free16kbAllocs.Full()) { Allocate256kb(pointer, size); return; }

	++allocations;
	*pointer = pool16kbPointer + free16kbAllocs.GetFree();
}

void Memory::Allocate256kb(void** pointer, U64 size)
{
	if (free256kbAllocs.Full()) { Allocate4mb(pointer, size); return; }

	++allocations;
	*pointer = pool256kbPointer + free256kbAllocs.GetFree();
}

void Memory::Allocate4mb(void** pointer, U64 size)
{
	if (free4mbAllocs.Full()) { *pointer = LargeAllocate(size); return; }

	++allocations;
	*pointer = pool4mbPointer + free4mbAllocs.GetFree();
}

void* Memory::LargeAllocate(U64 size)
{
	return calloc(1, size);
}

void* Memory::LargeReallocate(void** pointer, U64 size)
{
	return realloc(*pointer, size);
}

void Memory::FreeChunk(void** pointer)
{
	void* cmp = (void*)*pointer;

	if (cmp >= pool4mbPointer) { Free4mb((void**)pointer); }
	else if (cmp >= pool256kbPointer) { Free256kb((void**)pointer); }
	else if (cmp >= pool16kbPointer) { Free16kb((void**)pointer); }
	else if (cmp >= pool1kbPointer) { Free1kb((void**)pointer); }
}

void Memory::CopyFree(U8** pointer, U8* copy, U64 size)
{
	void* cmp = *pointer;

	if (cmp >= pool4mbPointer) { Copy(copy, *pointer, size); Free4mb((void**)pointer); }
	else if (cmp >= pool256kbPointer) { Copy(copy, *pointer, size); Free256kb((void**)pointer); }
	else if (cmp >= pool16kbPointer) { Copy(copy, *pointer, size); Free16kb((void**)pointer); }
	else if (cmp >= pool1kbPointer) { Copy(copy, *pointer, size); Free1kb((void**)pointer); }
}

void Memory::Free1kb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region1kb));
	free1kbAllocs.Release((U32)((Region1kb*)*pointer - pool1kbPointer));
	*pointer = nullptr;
}

void Memory::Free16kb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region16kb));
	free16kbAllocs.Release((U32)((Region16kb*)*pointer - pool16kbPointer));
	*pointer = nullptr;
}

void Memory::Free256kb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region256kb));
	free256kbAllocs.Release((U32)((Region256kb*)*pointer - pool256kbPointer));
	*pointer = nullptr;
}

void Memory::Free4mb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region4mb));
	free4mbAllocs.Release((U32)((Region4mb*)*pointer - pool4mbPointer));
	*pointer = nullptr;
}

void Memory::LargeFree(void** pointer)
{
	free(*pointer);
	*pointer = nullptr;
}

bool Memory::IsDynamicallyAllocated(void* pointer)
{
	static const void* upperBound = memory + totalSize;

	return pointer != nullptr && pointer >= pool1kbPointer && pointer < upperBound;
}

bool Memory::IsStaticallyAllocated(void* pointer)
{
	return pointer != nullptr && pointer >= memory && pointer < pool1kbPointer;
}

/*---------GLOBAL NEW/DELETE---------*/

NH_NODISCARD void* operator new(U64 size)
{
	if (size == 0) { return nullptr; }
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

NH_NODISCARD void* operator new[](U64 size)
{
	if (size == 0) { return nullptr; }
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

NH_NODISCARD void* operator new(U64 size, Align alignment)
{
	if (size == 0) { return nullptr; }
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

NH_NODISCARD void* operator new[](U64 size, Align alignment)
{
	if (size == 0) { return nullptr; }
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

void operator delete(void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }