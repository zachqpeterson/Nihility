#include "DynamicAllocator.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"

DynamicAllocator::DynamicAllocator(U64 size) : memory{ nullptr }, totalSize{ 0 }, memoryOffset{ 0 }, linearOffset{ 0 }
{
	if (size)
	{
		totalSize = size;
		memory = Platform::Allocate(size, false);
		Platform::Zero(memory, size);
		U64 largeSize = size >> 1;
		U64 linearSize = totalSize - largeSize;
		memoryOffset = allocations.Create(largeSize, memory);
		linearOffset = size - linearSize;
	}
}

DynamicAllocator::~DynamicAllocator()
{
	Destroy();
}

void DynamicAllocator::Destroy()
{
	allocations.Destroy();

	if (memory)
	{
		linearOffset = 0;
		Platform::Free(memory, false);
	}
}

void* DynamicAllocator::operator new(U64 size)
{
	return Platform::Allocate(sizeof(DynamicAllocator), false);
}

void DynamicAllocator::operator delete(void* ptr)
{
	Platform::Free(ptr, false);
}

bool DynamicAllocator::Allocate(void** ptr, U64 size)
{
	*ptr = (U8*)memory + allocations.AllocateBlock(size) + memoryOffset;
	return true;
}

bool DynamicAllocator::Free(void* block, U64 size)
{
	U64 offset = (U8*)block - (U8*)memory - memoryOffset;
	block = nullptr;

	return allocations.FreeBlock(size, offset);
}

void* DynamicAllocator::LinearAllocate(U64 size)
{
	void* ptr = (U8*)memory + linearOffset;
	linearOffset += size;

	return ptr;
}