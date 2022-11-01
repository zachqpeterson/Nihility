#pragma once

#include "Defines.hpp"

#include "Platform/Platform.hpp"

#define MAX_ALLOCATIONS 65536

struct NH_API Freelist
{
	struct Node
	{
		Node() : size{ U64_MAX }, offset{ U64_MAX }, next{ nullptr } {}

		U64 size;
		U64 offset;
		Node* next;
	};

public:
	Freelist() : totalSize{ 0 }, freeSpace{ 0 }, nodes{ nullptr }, head{ nullptr }, good{ false } {}
	Freelist(U64 size);
	void Create(U64 size);

	U64 AllocateBlock(U64 size);
	bool FreeBlock(U64 size, U64 offset);
	bool Resize(U64 size);
	//TODO: Defragment

	U64 TotalSize() const { return totalSize; }
	U64 FreeSpace() const { return freeSpace; }

private:
	U64 Create(U64 size, void* memory);

	Node* GetNode();

	U64 totalSize;
	U64 freeSpace;

	Node* nodes;
	Node* head;

	bool good;

	Freelist(const Freelist& other) = delete;
	Freelist& operator=(const Freelist& other) = delete;

	friend class DynamicAllocator;
};