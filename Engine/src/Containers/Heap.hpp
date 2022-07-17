#pragma once

#include "Defines.hpp"

#include "Memory/Memory.hpp"

struct Heap
{
private:
	struct Chunk { Chunk* next; };

public:
	void* Alloc(U64 size);
	void Free(void* p);
	void Cleanup();

	Chunk* head;
	void* firstFree;
	I32 remaining;
	U64 size;
};

void* Heap::Alloc(U64 size)
{
	this->size = size;

	if (firstFree)
	{
		void* p = firstFree;
		firstFree = *(void**)p;
		return p;
	}
	else
	{
		if (remaining == 0)
		{
			I32 count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
			Chunk* c = (Chunk*)Memory::Allocate(sizeof(Chunk) + size * count, MEMORY_TAG_DATA_STRUCT);
			if (c == NULL) { return NULL; }

			c->next = head;
			head = c;
			remaining = count;
		}

		--remaining;
		return (U8*)(head)+sizeof(Chunk) + size * remaining;
	}
}

void Heap::Free(void* p)
{
	*(void**)p = firstFree;
	firstFree = p;
}

void Heap::Cleanup()
{
	I32 count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
	Memory::Free(head, sizeof(Chunk) + size * count, MEMORY_TAG_DATA_STRUCT);
}