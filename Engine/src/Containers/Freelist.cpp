#include "Freelist.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"
#include "Containers/String.hpp"

Freelist::Freelist(U32 size) : totalSize{ size }, freeSpace{ totalSize }, head{ nullptr } {}

Freelist::Freelist(Freelist&& other) : totalSize{ other.totalSize }, freeSpace{ other.freeSpace }, head{ other.head }
{
    other.totalSize = 0;
    other.freeSpace = 0;
    other.head = nullptr;
}

Freelist::~Freelist()
{
    Destroy();
}

Freelist& Freelist::operator=(Freelist&& other)
{
    totalSize = other.totalSize;
    freeSpace = other.freeSpace;
    head = other.head;
    other.totalSize = 0;
    other.freeSpace = 0;
    other.head = nullptr;

    return *this;
}

void Freelist::Destroy()
{
    Node* node = head;
    while(node)
    {
        Node* temp = node;
        delete node;
        node = temp;
    }

    head = nullptr;
}

U32 Freelist::AllocateBlock(U32 size)
{
    if (size > freeSpace)
    {
        Logger::Error("Freelist::AllocateBlock: Not enough space to allocate {} bytes, space left: {}", size, freeSpace);
        return U32_MAX;
    }

    if(head == nullptr)
    {
        head = new Node(size, 0);
        freeSpace -= size;
        return 0;
    }

    Node* node = head;
    U32 offset = head->size;

    while (offset < U32_MAX)
    {
        if(!node->next)
        {
            node->next = new Node(size, offset);
            freeSpace -= size;

            return offset;
        }

        if ((node->next->offset - node->size) >= size)
        {
            node->next = new Node(size, offset, node->next);
            freeSpace -= size;

            return offset;
        }

        offset += node->size;
        node = node->next;
        continue;
    }

    Logger::Error("Freelist::AllocateBlock: No section large enough to take {} bytes, must defragment", size);
    return U32_MAX;
}

bool Freelist::FreeBlock(U32 size, U32 offset)
{
    if(offset == 0)
    {
        Node* temp = head->next;
        delete head;
        head = temp;
        freeSpace += size;
        
        return true;
    }

    Node* node = head->next;
    Node* prev = head;

    while(node)
    {
        if(node->offset == offset)
        {
            if(node->size != size)
            {
                Logger::Error("Freelist::FreeBlock: Memory block at offset {} is not of size {}!", offset, size);
                return false;
            }

            Node* temp = node->next;
            delete node;
            prev->next = temp;
            freeSpace += size;

            return true;
        }

        prev = node;
        node = node->next;
    }

    Logger::Error("Freelist::FreeBlock: There is no memory block at offset {}", offset);
    return false;
}

bool Freelist::Resize(U32 size)
{
    if((totalSize - freeSpace) > size)
    {
        Logger::Error("Freelist::Resize: Can't resize to a size smaller than allocated size: {}", totalSize - freeSpace);
        return false;
    }

    totalSize = size;
    return true;
}