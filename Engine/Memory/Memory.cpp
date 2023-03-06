#include "Memory.hpp"

#include "Platform\Jobs.hpp"
#include "Platform\ThreadSafety.hpp"

#include <stdlib.h>

U8* Memory::memory;
U64 Memory::totalSize;

U64 Memory::staticSize;
U8* Memory::staticPointer;

Memory::Region1kb* Memory::pool1kbPointer;
U32* Memory::free1kbIndices;
I64 Memory::last1kbFree;

Memory::Region16kb* Memory::pool16kbPointer;
U32* Memory::free16kbIndices;
I64 Memory::last16kbFree;

Memory::Region256kb* Memory::pool256kbPointer;
U32* Memory::free256kbIndices;
I64 Memory::last256kbFree;

Memory::Region4mb* Memory::pool4mbPointer;
U32* Memory::free4mbIndices;
I64 Memory::last4mbFree;

bool Memory::initialized = false;

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

		if (!memory) { return false; } //TODO: Log

		free1kbIndices = (U32*)memory;
		for (U32 i = 0; i < region1kbCount; ++i) { free1kbIndices[i] = i; }
		free16kbIndices = free1kbIndices + region1kbCount;
		for (U32 i = 0; i < region16kbCount; ++i) { free16kbIndices[i] = i; }
		free256kbIndices = free16kbIndices + region16kbCount;
		for (U32 i = 0; i < region256kbCount; ++i) { free256kbIndices[i] = i; }
		free4mbIndices = free256kbIndices + region256kbCount;
		for (U32 i = 0; i < region4mbCount; ++i) { free4mbIndices[i] = i; }

		pool1kbPointer = (Region1kb*)(free4mbIndices + region4mbCount);
		last1kbFree = 0;

		pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
		last16kbFree = 0;

		pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
		last256kbFree = 0;

		pool4mbPointer = (Region4mb*)(pool256kbPointer + region256kbCount);
		last4mbFree = 0;

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
	memset(*pointer, 0, sizeof(Region1kb));
	free1kbIndices[SafeDecrement(&last1kbFree)] = U32((Region1kb*)*pointer - pool1kbPointer);
	*pointer = nullptr;
}

void Memory::Free16kb(void** pointer)
{
	memset(*pointer, 0, sizeof(Region16kb));
	free16kbIndices[SafeDecrement(&last16kbFree)] = U32((Region16kb*)*pointer - pool16kbPointer);
	*pointer = nullptr;
}

void Memory::Free256kb(void** pointer)
{
	memset(*pointer, 0, sizeof(Region256kb));
	free256kbIndices[SafeDecrement(&last256kbFree)] = U32((Region256kb*)*pointer - pool256kbPointer);
	*pointer = nullptr;
}

void Memory::Free4mb(void** pointer)
{
	memset(*pointer, 0, sizeof(Region4mb));
	free4mbIndices[SafeDecrement(&last4mbFree)] = U32((Region4mb*)*pointer - pool4mbPointer);
	*pointer = nullptr;
}