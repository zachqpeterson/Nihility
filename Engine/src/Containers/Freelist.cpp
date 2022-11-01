#include "Freelist.hpp"

#include "Core/Logger.hpp"
#include "Containers/String.hpp"

Freelist::Freelist(U64 size) : totalSize{ size }, freeSpace{ size },
nodes{ (Node*)Memory::Allocate(sizeof(Node) * MAX_ALLOCATIONS, MEMORY_TAG_DATA_STRUCT) }, head{ nodes }, good{ true }, allocated{ true }
{
	head->size = totalSize;
	head->offset = 0;
	head->next = nullptr;

	for (U64 i = 1; i < MAX_ALLOCATIONS; ++i)
	{
		nodes[i].offset = U64_MAX;
		nodes[i].size = U64_MAX;
		nodes[i].next = nullptr;
	}
}

void Freelist::Create(U64 size)
{
	totalSize = size;
	freeSpace = size;
	nodes = (Node*)Memory::Allocate(sizeof(Node) * MAX_ALLOCATIONS, MEMORY_TAG_DATA_STRUCT);

	head = nodes;
	head->size = totalSize;
	head->offset = 0;
	head->next = nullptr;
	good = true;
	allocated = true;

	for (U64 i = 1; i < MAX_ALLOCATIONS; ++i)
	{
		nodes[i].offset = U64_MAX;
		nodes[i].size = U64_MAX;
		nodes[i].next = nullptr;
	}
}

U64 Freelist::Create(U64 size, void* memory)
{
	U64 offset = sizeof(Node) * MAX_ALLOCATIONS;

	totalSize = size - offset;
	freeSpace = totalSize;
	nodes = (Node*)memory;

	head = nodes;
	head->size = totalSize;
	head->offset = 0;
	head->next = nullptr;
	good = true;
	allocated = false;

	for (U64 i = 1; i < MAX_ALLOCATIONS; ++i)
	{
		nodes[i].offset = U64_MAX;
		nodes[i].size = U64_MAX;
		nodes[i].next = nullptr;
	}

	return offset;
}

Freelist::~Freelist()
{
	Destroy();
}

void Freelist::Destroy()
{
	if (allocated)
	{
		Memory::Free(nodes, sizeof(Node) * MAX_ALLOCATIONS, MEMORY_TAG_DATA_STRUCT);
	}

	head = nullptr;
	totalSize = 0;
	freeSpace = 0;
	good = false;
	allocated = false;
}

U64 Freelist::AllocateBlock(U64 size)
{
	if (size > freeSpace || !good)
	{
		Logger::Error("Freelist::AllocateBlock: Not enough space to allocate {} bytes, space left: {}", size, freeSpace);
		return 0;
	}

	Node* node = head;
	Node* previous = nullptr;
	while (node)
	{
		if (node->size == size)
		{
			U64 offset = node->offset;

			if (previous)
			{
				previous->next = node->next;
				node->next = nullptr;
				node->size = U64_MAX;
				node->offset = U64_MAX;
			}
			else
			{
				Node* prevHead = head;
				head = node->next;
				prevHead->next = nullptr;
				prevHead->size = U64_MAX;
				prevHead->offset = U64_MAX;
			}

			return offset;
		}
		else if (node->size > size)
		{
			U64 offset = node->offset;
			node->size -= size;
			node->offset += size;
			return offset;
		}

		previous = node;
		node = node->next;
	}

	Logger::Error("Freelist::AllocateBlock: No section large enough to take {} bytes, must defragment", size);
	return U64_MAX;
}

bool Freelist::FreeBlock(U64 size, U64 offset)
{
	if (!good) { return false; }

	Node* node = head;
	Node* previous = nullptr;

	if (!node)
	{
		Node* newNode = GetNode();
		newNode->offset = offset;
		newNode->size = size;
		newNode->next = nullptr;
		head = newNode;
	}
	else
	{
		while (node)
		{
			if (node->offset == offset)
			{
				node->size += size;

				if (node->next && node->next->offset == node->offset + node->size)
				{
					node->size += node->next->size;
					Node* next = node->next;
					node->next = node->next->next;
					next->size = U64_MAX;
					next->offset = U64_MAX;
					next->next = nullptr;
				}

				return true;
			}
			else if (node->offset > offset)
			{
				Node* newNode = GetNode();
				newNode->offset = offset;
				newNode->size = size;

				if (previous)
				{
					previous->next = newNode;
					newNode->next = node;
				}
				else
				{
					newNode->next = node;
					head = newNode;
				}

				if (newNode->next && newNode->offset + newNode->size == newNode->next->offset)
				{
					newNode->size += newNode->next->size;
					Node* rubbish = newNode->next;
					newNode->next = rubbish->next;
					rubbish->size = U64_MAX;
					rubbish->offset = U64_MAX;
					rubbish->next = nullptr;
				}

				if (previous && previous->offset + previous->size == newNode->offset)
				{
					previous->size += newNode->size;
					Node* rubbish = newNode;
					previous->next = rubbish->next;
					rubbish->size = U64_MAX;
					rubbish->offset = U64_MAX;
					rubbish->next = nullptr;
				}

				return true;
			}

			previous = node;
			node = node->next;
		}
	}

	Logger::Error("Freelist::FreeBlock: There is no memory block at offset {}", offset);
	return false;
}

bool Freelist::Resize(U64 size)
{
	U64 allocated = totalSize - freeSpace;
	if (allocated > size)
	{
		Logger::Error("Freelist::Resize: Can't resize to a size smaller than allocated size: {}", allocated);
		return false;
	}

	totalSize = size;
	freeSpace = totalSize - allocated;
	return true;
}

Freelist::Node* Freelist::GetNode()
{
	for (U64 i = 0; i < MAX_ALLOCATIONS; ++i)
	{
		if (nodes[i].offset == U64_MAX) { return nodes + i; }
	}

	return nullptr;
}