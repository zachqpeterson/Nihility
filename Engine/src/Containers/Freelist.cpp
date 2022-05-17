#include "Freelist.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"
#include "Containers/String.hpp"

Freelist::Freelist(U64 size)
{
    totalSize = size;
    maxEntries = (size / (sizeof(void*) * sizeof(Node)));
    nodes = (Node*)Memory::Allocate(sizeof(Node) * size, MEMORY_TAG_DATA_STRUCT);
    Memory::Zero(nodes, sizeof(Node) * size);

    head = &nodes[0];
    head->offset = 0;
    head->size = size;
    head->next = nullptr;

    for (U64 i = 1; i < maxEntries; ++i)
    {
        nodes[i].offset = INVALID_ID;
        nodes[i].size = INVALID_ID;
    }
}

void Freelist::Destroy()
{
    Memory::Free(nodes, sizeof(Node) * totalSize, MEMORY_TAG_DATA_STRUCT);
    nodes = nullptr;
    head = nullptr;
}

bool Freelist::AllocateBlock(U64 size, U64* outOffset)
{
    Node* node = head;
    Node* previous = nullptr;
    while (node)
    {
        if (node->size == size)
        {
            *outOffset = node->offset;
            Node* nodeToReturn = nullptr;
            if (previous)
            {
                previous->next = node->next;
                nodeToReturn = node;
            }
            else
            {
                nodeToReturn = head;
                head = node->next;
            }
            ReturnNode(nodeToReturn);
            return true;
        }
        else if (node->size > size)
        {
            *outOffset = node->offset;
            node->size -= size;
            node->offset += size;
            return true;
        }

        previous = node;
        node = node->next;
    }

    U64 freeSpace = FreeSpace();
    LOG_WARN("freelist_find_block, no block with enough free space found (requested: %lluB, available: %lluB).", size, freeSpace);
    return false;
}

bool Freelist::FreeBlock(U64 size, U64 offset)
{
    Node* node = head;
    Node* previous = nullptr;
    if (!node)
    {
        Node* newNode = GetNode();
        newNode->offset = offset;
        newNode->size = size;
        newNode->next = nullptr;
        head = newNode;
        return true;
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
                    ReturnNode(next);
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
                    ReturnNode(rubbish);
                }

                if (previous && previous->offset + previous->size == newNode->offset)
                {
                    previous->size += newNode->size;
                    Node* rubbish = newNode;
                    previous->next = rubbish->next;
                    ReturnNode(rubbish);
                }

                return true;
            }

            previous = node;
            node = node->next;
        }
    }

    LOG_WARN("Unable to find block to be freed. Corruption possible?");
    return false;
}

bool Freelist::Resize(U64 newSize)
{
    Node* temp = nodes;
    U64 oldSize = totalSize;
    U64 sizeDiff = newSize - totalSize;

    nodes = (Node*)Memory::Allocate(sizeof(Node) * newSize, MEMORY_TAG_DATA_STRUCT);

    Memory::Zero(nodes, sizeof(Node) * newSize);

    maxEntries = (newSize / (sizeof(void*) * sizeof(Node)));
    totalSize = newSize;

    for (U64 i = 1; i < maxEntries; ++i)
    {
        nodes[i].offset = INVALID_ID;
        nodes[i].size = INVALID_ID;
    }

    head = &nodes[0];

    Node* newListNode = head;
    Node* oldNode = &temp[0];
    if (!oldNode)
    {
        head->offset = oldSize;
        head->size = sizeDiff;
        head->next = 0;
    }
    else
    {
        while (oldNode)
        {
            Node* newNode = GetNode();
            newNode->offset = oldNode->offset;
            newNode->size = oldNode->size;
            newNode->next = 0;
            newListNode->next = newNode;
            newListNode = newListNode->next;

            if (oldNode->next)
            {
                oldNode = oldNode->next;
            }
            else
            {
                if (oldNode->offset + oldNode->size == oldSize)
                {
                    newNode->size += sizeDiff;
                }
                else
                {
                    Node* newNodeEnd = GetNode();
                    newNodeEnd->offset = oldSize;
                    newNodeEnd->size = sizeDiff;
                    newNodeEnd->next = 0;
                    newNode->next = newNodeEnd;
                }
                break;
            }
        }
    }

    Memory::Free(temp, oldSize, MEMORY_TAG_DATA_STRUCT);

    return true;
}

void Freelist::Cleanup()
{
    for (U64 i = 1; i < maxEntries; ++i)
    {
        nodes[i].offset = INVALID_ID;
        nodes[i].size = INVALID_ID;
    }

    head->offset = 0;
    head->size = totalSize;
    head->next = nullptr;
}

bool Freelist::FreeSpace()
{
    U64 runningTotal = 0;
    Node* node = head;

    while (node)
    {
        runningTotal += node->size;
        node = node->next;
    }

    return runningTotal;
}

Freelist::Node* Freelist::GetNode()
{
    for (U64 i = 1; i < maxEntries; ++i)
    {
        if (nodes[i].offset == INVALID_ID)
        {
            return &nodes[i];
        }
    }

    return nullptr;
}

void Freelist::ReturnNode(Node* node)
{
    node->offset = INVALID_ID;
    node->size = INVALID_ID;
    node->next = 0;
}