#include "Freelist.hpp"

import ThreadSafety;

#include "Memory\Memory.hpp"

Freelist::Freelist(NullPointer) : capacity{ capacity }, used{ used }, outsideAllocated{ outsideAllocated }, freeCount{ freeCount }, freeIndices{ freeIndices }, lastFree{ lastFree } {}

Freelist::Freelist() {}

Freelist::Freelist(U32 count) : capacity{ count }
{
	Memory::AllocateArray(&freeIndices, count);
}

Freelist& Freelist::operator()(U32 count)
{
	if (freeIndices)
	{
		Resize(count);

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
	if (freeIndices && !outsideAllocated) { Memory::Free(&freeIndices); }

	freeCount = 0;
	lastFree = 0;
	capacity = count;
	freeIndices = memory;
	outsideAllocated = true;

	return *this;
}

Freelist::~Freelist()
{
	Destroy();
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

void Freelist::Reset()
{
	freeCount = 0;
	lastFree = 0;
	used = 0;
	Memory::Zero(freeIndices, sizeof(U32) * capacity);
}

U32 Freelist::GetFree()
{
	if (Full()) { return U32_MAX; }

	U32 index = SafeDecrement(&freeCount);

	if (index < capacity) { ++used; return freeIndices[index]; }

	++used;
	++freeCount;
	return SafeIncrement(&lastFree) - 1;
}

void Freelist::Release(U32 index)
{
	--used;
	freeIndices[SafeIncrement(&freeCount) - 1] = index;
}

bool Freelist::Full() const
{
	return lastFree >= capacity && freeCount == 0;
}

U32 Freelist::Size() const
{
	return used;
}

U32 Freelist::Capacity() const
{
	return capacity;
}

U32 Freelist::Last() const
{
	return lastFree;
}

void Freelist::Resize(U32 count)
{
	//TODO: Make thread safe

	if (count <= capacity) { return; }

	if (outsideAllocated)
	{
		U32* temp = freeIndices;
		Memory::AllocateArray(&freeIndices, count);
		Memory::Copy(freeIndices, temp, sizeof(U32) * count);

		freeCount += capacity - count;
		capacity = count;
		outsideAllocated = false;
	}
	else
	{
		Memory::Reallocate(&freeIndices, count);

		freeCount += capacity - count;
		capacity = count;
	}
}