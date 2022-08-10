#include "Freelist.hpp"

#include "Core/Logger.hpp"
#include "Containers/String.hpp"

Freelist::Freelist(U64 size) : totalSize{ size }, freeSpace{ totalSize }, head{ nullptr } {}

Freelist::Freelist(Freelist&& other) noexcept : totalSize{ other.totalSize }, freeSpace{ other.freeSpace }, head{ other.head }
{
    other.totalSize = 0;
    other.freeSpace = 0;
    other.head = nullptr;
}

Freelist::~Freelist()
{
    Destroy();
}

void Freelist::Destroy()
{
    while(head)
    {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
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

U64 Freelist::AllocateBlock(U64 size)
{
    if (size > freeSpace)
    {
        Logger::Error("Freelist::AllocateBlock: Not enough space to allocate {} bytes, space left: {}", size, freeSpace);
        return U64_MAX;
    }

    if(head == nullptr || size <= head->offset)
    {
        head = new Node(size, 0, head);
        freeSpace -= size;
        return 0;
    }

    Node* node = head;
    U64 offset = head->size;

    while (offset < U64_MAX && node->next && (node->next->offset - node->size + node->offset) < size)
    {
        offset = node->next->size + node->next->offset;
        node = node->next;
    }

    if (!node->next || (node->next->offset - node->size + node->offset) >= size)
    {
        node->next = new Node(size, offset, node->next);
        freeSpace -= size;

        return offset;
    }

    Logger::Error("Freelist::AllocateBlock: No section large enough to take {} bytes, must defragment", size);
    return U64_MAX;
}

bool Freelist::FreeBlock(U64 size, U64 offset)
{
    if(offset == head->offset)
    {
        Node* temp = head->next;
        delete head;
        head = temp;
        freeSpace += size;
        
        return true;
    }

    Node* node = head->next;
    Node* prev = head;

    while(node && node->offset < offset)
    {
        prev = node;
        node = node->next;
    }

    if (node && node->offset == offset)
    {
        if (node->size != size)
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