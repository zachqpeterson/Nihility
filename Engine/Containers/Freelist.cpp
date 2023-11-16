#include "Freelist.hpp"

#include "Memory\Memory.hpp"
#include "Platform\ThreadSafety.hpp"

Freelist::Freelist() {}

Freelist::Freelist(U32 count) : capacity{ count }
{
	freeCount = 0;
	lastFree = 0;
	outsideAllocated = false;

	Memory::AllocateArray(&freeIndices, count);
}

Freelist& Freelist::operator()(U32 count)
{
	if (freeIndices)
	{
		//TODO: Resize

		return *this;
	}

	freeCount = 0;
	lastFree = 0;
	capacity = count;
	Memory::AllocateArray(&freeIndices, count);
	outsideAllocated = false;

	return *this;
}

Freelist& Freelist::operator()(U32* memory, U32 count)
{
	if (freeIndices)
	{
		//TODO: Resize

		return *this;
	}

	freeCount = 0;
	lastFree = 0;
	capacity = count;
	freeIndices = memory;
	outsideAllocated = true;

	return *this;
}

void Freelist::Destroy()
{
	capacity = 0;
	freeCount = 0;
	lastFree = 0;

	if (freeIndices && !outsideAllocated) { Memory::Free(&freeIndices); }

	freeIndices = nullptr;
	outsideAllocated = false;
}

Freelist::~Freelist()
{
	Destroy();
}

void Freelist::Reset()
{
	freeCount = 0;
	lastFree = 0;
	Memory::Zero(freeIndices, sizeof(U32) * capacity);
}

U32 Freelist::GetFree()
{
	U32 index = SafeDecrement(&freeCount);

	if (index < capacity) { return freeIndices[index]; }

	++freeCount;
	return SafeIncrement(&lastFree) - 1;
}

void Freelist::Release(U32 index)
{
	freeIndices[SafeIncrement(&freeCount) - 1] = index;
}

bool Freelist::Full() const
{
	return lastFree >= capacity && freeCount == 0;
}