#include "Memory.hpp"

#include <string.h>

U8* Memory::memory;
U64 Memory::totalSize;

U64 Memory::staticSize;
U8* Memory::staticPointer;

U32* Memory::free1kbIndices;
I64 Memory::last1kbFree{ 0 };

U32* Memory::free16kbIndices;
I64 Memory::last16kbFree{ 0 };

U32* Memory::free256kbIndices;
I64 Memory::last256kbFree{ 0 };

U32* Memory::free4mbIndices;
I64 Memory::last4mbFree{ 0 };

bool Memory::initialized{ false };

bool Memory::Initialize()
{
	if (!SafeCheckAndSet(&initialized))
	{
		U64 maxKilobytes = DYNAMIC_SIZE / 1024;

		U32 region4mbCount = U32(maxKilobytes / 81920);
		U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
		U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
		U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region4mbCount * 4096));

		U64 pointerToDynamic = sizeof(U32) * (region1kbCount + region16kbCount + region256kbCount + region4mbCount);

		totalSize = pointerToDynamic + DYNAMIC_SIZE + STATIC_SIZE;

		memory = (U8*)calloc(1, totalSize);

		if (!memory) { return false; }

		free1kbIndices = (U32*)memory;
		for (U32 i = 0; i < region1kbCount; ++i) { free1kbIndices[i] = i; }
		free16kbIndices = free1kbIndices + region1kbCount;
		for (U32 i = 0; i < region16kbCount; ++i) { free16kbIndices[i] = i; }
		free256kbIndices = free16kbIndices + region16kbCount;
		for (U32 i = 0; i < region256kbCount; ++i) { free256kbIndices[i] = i; }
		free4mbIndices = free256kbIndices + region256kbCount;
		for (U32 i = 0; i < region4mbCount; ++i) { free4mbIndices[i] = i; }

		pool1kbPointer = (Region1kb*)(free4mbIndices + region4mbCount);
		pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
		pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
		pool4mbPointer = (Region4mb*)(pool256kbPointer + region256kbCount);

		staticPointer = (U8*)(pool4mbPointer + region4mbCount); //TODO: linear allocator should be before dynamic allocator
		staticSize = memory - staticPointer;
	}

	return true;
}

void Memory::Shutdown()
{
	free(memory);
}

void Memory::Allocate1kb(void** pointer)
{
	static bool init = Initialize();

	*pointer = pool1kbPointer + free1kbIndices[SafeIncrement(&last1kbFree) - 1];
}

void Memory::Allocate16kb(void** pointer)
{
	static bool init = Initialize();

	*pointer = pool16kbPointer + free16kbIndices[SafeIncrement(&last16kbFree) - 1];
}

void Memory::Allocate256kb(void** pointer)
{
	static bool init = Initialize();

	*pointer = pool256kbPointer + free256kbIndices[SafeIncrement(&last256kbFree) - 1];
}

void Memory::Allocate4mb(void** pointer)
{
	static bool init = Initialize();

	*pointer = pool4mbPointer + free4mbIndices[SafeIncrement(&last4mbFree) - 1];
}

void Memory::Free1kb(void** pointer)
{
	Zero(*pointer, sizeof(Region1kb));
	free1kbIndices[SafeDecrement(&last1kbFree)] = U32((Region1kb*)*pointer - pool1kbPointer);
	*pointer = nullptr;
}

void Memory::Free16kb(void** pointer)
{
	Zero(*pointer, sizeof(Region16kb));
	free16kbIndices[SafeDecrement(&last16kbFree)] = U32((Region16kb*)*pointer - pool16kbPointer);
	*pointer = nullptr;
}

void Memory::Free256kb(void** pointer)
{
	Zero(*pointer, sizeof(Region256kb));
	free256kbIndices[SafeDecrement(&last256kbFree)] = U32((Region256kb*)*pointer - pool256kbPointer);
	*pointer = nullptr;
}

void Memory::Free4mb(void** pointer)
{
	Zero(*pointer, sizeof(Region4mb));
	free4mbIndices[SafeDecrement(&last4mbFree)] = U32((Region4mb*)*pointer - pool4mbPointer);
	*pointer = nullptr;
}

U64 Memory::MemoryAlign(U64 size, U64 alignment)
{
	const U64 alignmentMask = alignment - 1;
	return (size + alignmentMask) & ~alignmentMask;
}

void Memory::Set(void* pointer, U8 value, U64 size)
{
	U8* ptr = (U8*)pointer;
	while (size) { --size; *ptr = value; ++ptr; }
}

void Memory::Zero(void* pointer, U64 size)
{
	static constexpr U8 ZERO = 0UI8;

	U8* ptr = (U8*)pointer;
	while (size) { --size; *ptr = ZERO; ++ptr; }
}

void Memory::Copy(void* dst, const void* src, U64 size)
{
	memcpy(dst, src, size);
}

/*---------GLOBAL NEW/DELETE---------*/

NH_NODISCARD void* operator new (U64 size)
{
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

NH_NODISCARD void* operator new[](U64 size)
{
	U8* ptr;
	Memory::AllocateSize(&ptr, size);
	return ptr;
}

void operator delete (void* ptr)
{
	Memory::FreeSize(&ptr);
}

void operator delete[](void* ptr)
{
	Memory::FreeSize(&ptr);
}