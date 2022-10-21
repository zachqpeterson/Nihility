#pragma once

#include "Defines.hpp"

#include "Containers/Freelist.hpp"

class NH_API DynamicAllocator
{
public:
	DynamicAllocator(U64 size);
	~DynamicAllocator();
	void Destroy();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	bool Allocate(void** ptr, U64 size);
	bool Free(void* block, U64 size);

	bool LinearAllocate(void** ptr, U64 size);

private:
	U64 totalSize;
	U64 memoryOffset;
	U64 linearOffset;

	Freelist allocations;
	void* memory;
};