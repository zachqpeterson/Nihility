#include "Memory.hpp"

#include "Platform\Jobs.hpp"
#include "Core\Logger.hpp"
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

Memory::Region1mb* Memory::pool1mbPointer;
U32* Memory::free1mbIndices;
I64 Memory::last1mbFree;

bool Memory::initialized = false;

bool Memory::Initialize()
{
	if (!SafeCheckAndSet(&initialized))
	{
		U64 maxKilobytes = DYNAMIC_SIZE / 1024;

		U32 region1mbCount = U32(maxKilobytes / 20480);
		U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
		U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
		U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region1mbCount * 1024));

		U64 pointerToDynamic = sizeof(U32) * (region1kbCount + region16kbCount + region256kbCount + region1mbCount);

		totalSize = pointerToDynamic + DYNAMIC_SIZE + STATIC_SIZE;

		memory = (U8*)calloc(1, totalSize);

		if (!memory) { return false; } //TODO: Log

		free1kbIndices = (U32*)memory;
		for (U32 i = 0; i < region1kbCount; ++i) { free1kbIndices[i] = i; }
		free16kbIndices = free1kbIndices + region1kbCount;
		for (U32 i = 0; i < region16kbCount; ++i) { free16kbIndices[i] = i; }
		free256kbIndices = free16kbIndices + region16kbCount;
		for (U32 i = 0; i < region256kbCount; ++i) { free256kbIndices[i] = i; }
		free1mbIndices = free256kbIndices + region256kbCount;
		for (U32 i = 0; i < region1mbCount; ++i) { free1mbIndices[i] = i; }

		pool1kbPointer = (Region1kb*)(free1mbIndices + region1mbCount);
		last1kbFree = 0;

		pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
		last16kbFree = 0;

		pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
		last256kbFree = 0;

		pool1mbPointer = (Region1mb*)(pool256kbPointer + region256kbCount);
		last1mbFree = 0;

		staticPointer = (U8*)(pool1mbPointer + region1mbCount); //TODO: linear allocator should be before dynamic allocator
		staticSize = memory - staticPointer;
	}

	return true;
}

void Memory::Shutdown()
{
	free(memory);
}

void* Memory::Allocate(U64 size)
{
	if (size <= 1024) { return Allocate1kb(); }
	else if (size <= 16384) { return Allocate16kb(); }
	else if (size <= 262144) { return Allocate256kb(); }
	else if (size <= 1048576) { return Allocate1mb(); }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, 1048576);

	return nullptr;
}

void* Memory::Allocate(U64 size, U64& outSize)
{
	if (size <= 1024) { outSize = 1024; return Allocate1kb(); }
	else if (size <= 16384) { outSize = 16384; return Allocate16kb(); }
	else if (size <= 262144) { outSize = 262144; return Allocate256kb(); }
	else if (size <= 1048576) { outSize = 1048576; return Allocate1mb(); }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, 1048576);

	return nullptr;
}

void Memory::Free(void* ptr)
{
	if (ptr >= pool1mbPointer) { Free1mb(ptr); }
	else if (ptr >= pool256kbPointer) { Free256kb(ptr); }
	else if (ptr >= pool16kbPointer) { Free16kb(ptr); }
	else if (ptr >= pool1kbPointer) { Free1kb(ptr); }
	else
	{
		BreakPoint;
		//TODO: error, too large
		//TODO: check if pointer is past pool1mb range
	}
}

void* Memory::Allocate1kb()
{
	static bool init = Initialize();

	return pool1kbPointer + free1kbIndices[SafeIncrement(&last1kbFree) - 1];
}

void* Memory::Allocate16kb()
{
	static bool init = Initialize();

	return pool16kbPointer + free16kbIndices[SafeIncrement(&last16kbFree) - 1];
}

void* Memory::Allocate256kb()
{
	static bool init = Initialize();

	return pool256kbPointer + free256kbIndices[SafeIncrement(&last256kbFree) - 1];
}

void* Memory::Allocate1mb()
{
	static bool init = Initialize();

	I64 i = SafeIncrement(&last1mbFree) - 1;
	return pool1mbPointer + free1mbIndices[i];
}

//TODO: Debug checking that ensures this is the right size block to free
void Memory::Free1kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region1kb));
	free1kbIndices[SafeDecrement(&last1kbFree)] = U32((Region1kb*)ptr - pool1kbPointer);
}

void Memory::Free16kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region16kb));
	free16kbIndices[SafeDecrement(&last16kbFree)] = U32((Region16kb*)ptr - pool16kbPointer);
}

void Memory::Free256kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region256kb));
	free256kbIndices[SafeDecrement(&last256kbFree)] = U32((Region256kb*)ptr - pool256kbPointer);
}

void Memory::Free1mb(void* ptr)
{
	memset(ptr, 0, sizeof(Region1mb));
	free1mbIndices[SafeDecrement(&last1mbFree)] = U32((Region1mb*)ptr - pool1mbPointer);
}

void* Memory::AllocateStatic(U64 size)
{
	static bool init = Initialize();

	if (staticPointer + size <= memory + totalSize)
	{
		U8* block = staticPointer;
		staticPointer += size;

		return block;
	}

	return nullptr;
}