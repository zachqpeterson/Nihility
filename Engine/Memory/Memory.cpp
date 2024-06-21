#include "Memory.hpp"

import ThreadSafety;

#include <corecrt_malloc.h>
#include <vcruntime_string.h>

U32 Memory::allocations{ 0 };
U8* Memory::memory{ nullptr };
U64 Memory::totalSize{ 0 };

U8* Memory::dynamicPointer{ nullptr };
U8* Memory::staticPointer{ nullptr };

Memory::Region1kb* Memory::pool1kbPointer{ nullptr };
Freelist Memory::free1kbIndices{ nullptr };

Memory::Region16kb* Memory::pool16kbPointer{ nullptr };
Freelist Memory::free16kbIndices{ nullptr };

Memory::Region256kb* Memory::pool256kbPointer{ nullptr };
Freelist Memory::free256kbIndices{ nullptr };

Memory::Region4mb* Memory::pool4mbPointer{ nullptr };
Freelist Memory::free4mbIndices{ nullptr };

bool Memory::initialized{ false };

bool Memory::Initialize()
{
	if (!SafeCheckAndSet(&initialized, 0L))
	{
		U64 maxKilobytes = DYNAMIC_MEMORY_SIZE / 1024;

		U32 region4mbCount = U32(maxKilobytes / 81920);
		U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
		U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
		U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region4mbCount * 4096));

		U32 freeListMemory = (region4mbCount + region256kbCount + region16kbCount + region1kbCount) * sizeof(U32);

		totalSize = DYNAMIC_MEMORY_SIZE + STATIC_MEMORY_SIZE;

		memory = (U8*)calloc(1, totalSize + freeListMemory);

		if (!memory) { return false; }

		staticPointer = memory;
		dynamicPointer = memory + STATIC_MEMORY_SIZE;

		pool1kbPointer = (Region1kb*)(dynamicPointer);
		pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
		pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
		pool4mbPointer = (Region4mb*)(pool256kbPointer + region256kbCount);

		U32* freeLists = (U32*)(memory + totalSize);

		free1kbIndices(freeLists, region1kbCount);
		free16kbIndices(freeLists + region1kbCount, region16kbCount);
		free256kbIndices(freeLists + region1kbCount + region16kbCount, region256kbCount);
		free4mbIndices(freeLists + region1kbCount + region16kbCount + region256kbCount, region4mbCount);
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
	if (free1kbIndices.Full()) { Allocate16kb(pointer, size); return; }

	++allocations;
	*pointer = pool1kbPointer + free1kbIndices.GetFree();
}

void Memory::Allocate16kb(void** pointer, U64 size)
{
	if (free16kbIndices.Full()) { Allocate256kb(pointer, size); return; }

	++allocations;
	*pointer = pool16kbPointer + free16kbIndices.GetFree();
}

void Memory::Allocate256kb(void** pointer, U64 size)
{
	if (free256kbIndices.Full()) { Allocate4mb(pointer, size); return; }

	++allocations;
	*pointer = pool256kbPointer + free256kbIndices.GetFree();
}

void Memory::Allocate4mb(void** pointer, U64 size)
{
	if (free4mbIndices.Full()) { *pointer = LargeAllocate(size); return; }

	++allocations;
	*pointer = pool4mbPointer + free4mbIndices.GetFree();
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

void Memory::CopyFree(void** pointer, void* copy, U64 size)
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
	free1kbIndices.Release((U32)((Region1kb*)*pointer - pool1kbPointer));
	*pointer = nullptr;
}

void Memory::Free16kb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region16kb));
	free16kbIndices.Release((U32)((Region16kb*)*pointer - pool16kbPointer));
	*pointer = nullptr;
}

void Memory::Free256kb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region256kb));
	free256kbIndices.Release((U32)((Region256kb*)*pointer - pool256kbPointer));
	*pointer = nullptr;
}

void Memory::Free4mb(void** pointer)
{
	if (!initialized) { return; }
	Zero(*pointer, sizeof(Region4mb));
	free4mbIndices.Release((U32)((Region4mb*)*pointer - pool4mbPointer));
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

U64 Memory::MemoryAlign(U64 size, U64 alignment)
{
	const U64 alignmentMask = alignment - 1;
	return (size + alignmentMask) & ~alignmentMask;
}

void Memory::Set(void* pointer, U8 value, U64 size)
{
	U8* ptr = (U8*)pointer;
	while (size--) { *ptr++ = value; }
}

void Memory::Zero(void* pointer, U64 size)
{
	static constexpr U8 ZERO = 0UI8;

	U8* ptr = (U8*)pointer;
	while (size--) { *ptr++ = ZERO; }
}

void Memory::Copy(void* dst, const void* src, U64 size)
{
	memcpy(dst, src, size);
}

void Memory::CopyRepeated(void* dst, const void* src, U64 size, U64 count)
{
	if (src > dst)
	{
		U8* it0 = (U8*)dst;
		const U8* start = (const U8*)src;
		const U8* it1;
		U64 s;

		for (U64 i = 0; i < count; ++i)
		{
			it1 = start;
			s = size;

			while (s--) { *it0++ = *it1++; }
		}
	}
	else
	{
		U8* it0 = (U8*)dst + size - 1;
		const U8* start = (const U8*)src + size - 1;
		const U8* it1;
		U64 s;

		for (U64 i = 0; i < count; ++i)
		{
			it1 = start;
			s = size;

			while (s--) { *it0-- = *it1--; }
		}
	}
}

void Memory::CopyRepeatedGap(void* dst, const void* src, U64 size, U64 count, U64 gap)
{
	if (src > dst)
	{
		U8* it0 = (U8*)dst;
		const U8* start = (const U8*)src;
		const U8* it1;
		U64 s;

		for (U64 i = 0; i < count; ++i)
		{
			it1 = start;
			s = size;

			while (s--) { *it0 = *it1++; it0 += gap; }
		}
	}
	else
	{
		U8* it0 = (U8*)dst + size - (size % gap);
		const U8* start = (const U8*)src + size - 1;
		const U8* it1;
		U64 s;

		for (U64 i = 0; i < count; ++i)
		{
			it1 = start;
			s = size;

			while (s--) { *it0 = *it1--; it0 -= gap; }
		}
	}
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