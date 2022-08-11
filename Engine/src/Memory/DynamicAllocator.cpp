#include "DynamicAllocator.hpp"

#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"

DynamicAllocator::DynamicAllocator(U64 size) : memory{ nullptr }, smallOffset{ 0 }
{
	if (size)
	{
		memory = Platform::Allocate(size, false);
		U64 largeSize = size * 0.9f;
		smallOffset = largeSize;
		allocations.Resize(largeSize);
		smallAllocations.Resize(size - largeSize);
	}
}

DynamicAllocator::~DynamicAllocator()
{
	Destroy();
}

void DynamicAllocator::Destroy()
{
	if (memory)
	{
		allocations.Destroy();
		smallAllocations.Destroy();
		smallOffset = 0;
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

DynamicAllocator& DynamicAllocator::operator=(DynamicAllocator&& other) noexcept
{
	memory = other.memory;
	allocations = Move(other.allocations);
	smallAllocations = Move(other.smallAllocations);
	smallOffset = other.smallOffset;
	other.memory = nullptr;

	return *this;
}

bool DynamicAllocator::Allocate(void** ptr, U64 size)
{
	if (size <= 32)
	{
		*ptr = (U8*)memory + smallAllocations.AllocateBlock(size) + smallOffset;
		return size > 0;
	}

	*ptr = (U8*)memory + allocations.AllocateBlock(size);
	return true;
}

bool DynamicAllocator::Free(void* block, U64 size)
{
	U64 offset = (U8*)block - (U8*)memory;
	block = nullptr;

	if (size <= 32) { return smallAllocations.FreeBlock(size, offset - smallOffset); }
	return allocations.FreeBlock(size, offset);
}