#include "Freelist.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"

#define INVALID_ID 4294967295U

struct Node {
    U64 offset;
    U64 size;
    Node* next;
};

struct State {
    U64 totalSize;
    U64 maxEntries;
    Node* head;
    Node* nodes;
};

Freelist::Freelist(U64 size, void* block) : memory{block}
{
    Memory::ZeroMemory(memory, GetMemoryRequirement(size));

    State* state = (State*)memory;
    state->nodes = (Node*)memory + sizeof(State);
    state->maxEntries = (size / (sizeof(void*) * sizeof(Node)));
    state->totalSize = size;

    state->head = &state->nodes[0];
    state->head->offset = 0;
    state->head->size = size;
    state->head->next = nullptr;

    for (U64 i = 1; i < state->maxEntries; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }
}

void Freelist::Destroy()
{
    State* state = (State*)memory;
    Memory::ZeroMemory(memory, sizeof(State) + sizeof(Node) * state->maxEntries);
    memory = nullptr;
}

bool Freelist::AllocateBlock(U64 size, U64* out_offset)
{
    State* state = (State*)memory;
    
    Node* node = state->head;
    Node* previous = nullptr;
    while (node) 
    {
        if (node->size == size) 
        {
            *out_offset = node->offset;
            Node* node_to_return = 0;
            if (previous) 
            {
                previous->next = node->next;
                node_to_return = node;
            }
            else 
            {
                node_to_return = state->head;
                state->head = node->next;
            }
            ReturnNode(node_to_return);
            return true;
        }
        else if (node->size > size) 
        {
            *out_offset = node->offset;
            node->size -= size;
            node->offset += size;
            return true;
        }

        previous = node;
        node = node->next;
    }

    U64 free_space = FreeSpace();
    WARN("freelist_find_block, no block with enough free space found (requested: %lluB, available: %lluB).", size, free_space);
    return false;
}

bool Freelist::FreeBlock(U64 size, U64 offset)
{
    State* state = (State*)memory;
    Node* node = state->head;
    Node* previous = nullptr;
    if (!node) 
    {
        // Check for the case where the entire thing is allocated.
        // In this case a new node is needed at the head.
        Node* new_node = GetNode();
        new_node->offset = offset;
        new_node->size = size;
        new_node->next = nullptr;
        state->head = new_node;
        return true;
    }
    else 
    {
        while (node) 
        {
            if (node->offset == offset) 
            {
                // Can just be appended to this node.
                node->size += size;

                // Check if this then connects the range between this and the next
                // node, and if so, combine them and return the second node..
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
                // Iterated beyond the space to be freed. Need a new node.
                Node* new_node = GetNode();
                new_node->offset = offset;
                new_node->size = size;

                // If there is a previous node, the new node should be inserted between this and it.
                if (previous) 
                {
                    previous->next = new_node;
                    new_node->next = node;
                }
                else 
                {
                    // Otherwise, the new node becomes the head.
                    new_node->next = node;
                    state->head = new_node;
                }

                // Double-check next node to see if it can be joined.
                if (new_node->next && new_node->offset + new_node->size == new_node->next->offset) 
                {
                    new_node->size += new_node->next->size;
                    Node* rubbish = new_node->next;
                    new_node->next = rubbish->next;
                    ReturnNode(rubbish);
                }

                // Double-check previous node to see if the new_node can be joined to it.
                if (previous && previous->offset + previous->size == new_node->offset) 
                {
                    previous->size += new_node->size;
                    Node* rubbish = new_node;
                    previous->next = rubbish->next;
                    ReturnNode(rubbish);
                }

                return true;
            }

            previous = node;
            node = node->next;
        }
    }

    WARN("Unable to find block to be freed. Corruption possible?");
    return false;
}

bool Freelist::Resize(void* new_memory, U64 newSize, void** out_old_memory)
{
    // Assign the old memory pointer so it can be freed.
    *out_old_memory = memory;

    // Copy over the old state to the new.
    State* old_state = (State*)memory;
    U64 size_diff = newSize - old_state->totalSize;

    // Setup the new memory
    memory = new_memory;

    // The block's layout is head* first, then array of available nodes.
    Memory::ZeroMemory(memory, GetMemoryRequirement(newSize));

    // Setup the new state.
    State* state = (State*)memory;
    state->nodes = (Node*)memory + sizeof(State);
    state->maxEntries = (newSize / (sizeof(void*) * sizeof(Node)));
    state->totalSize = newSize;

    // Invalidate the offset and size for all but the first node. The invalid
    // value will be checked for when seeking a new node from the list.
    for (U64 i = 1; i < state->maxEntries; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }

    state->head = &state->nodes[0];

    // Copy over the nodes.
    Node* new_list_node = state->head;
    Node* old_node = old_state->head;
    if (!old_node) {
        // If there is no head, then the entire list is allocated. In this case,
        // the head should be set to the difference of the space now available, and
        // at the end of the list.
        state->head->offset = old_state->totalSize;
        state->head->size = size_diff;
        state->head->next = 0;
    }
    else {
        // Iterate the old nodes.
        while (old_node) {
            // Get a new node, copy the offset/size, and set next to it.
            Node* new_node = GetNode();
            new_node->offset = old_node->offset;
            new_node->size = old_node->size;
            new_node->next = 0;
            new_list_node->next = new_node;
            // Move to the next entry.
            new_list_node = new_list_node->next;

            if (old_node->next) {
                // If there is another node, move on.
                old_node = old_node->next;
            }
            else {
                // Reached the end of the list.
                // Check if it extends to the end of the block. If so,
                // just append to the size. Otherwise, create a new node and
                // attach to it.
                if (old_node->offset + old_node->size == old_state->totalSize) {
                    new_node->size += size_diff;
                }
                else {
                    Node* new_node_end = GetNode();
                    new_node_end->offset = old_state->totalSize;
                    new_node_end->size = size_diff;
                    new_node_end->next = 0;
                    new_node->next = new_node_end;
                }
                break;
            }
        }
    }

    return true;
}

void Freelist::Clear()
{
    State* state = (State*)memory;
    
    for (U64 i = 1; i < state->maxEntries; ++i) 
    {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }

    state->head->offset = 0;
    state->head->size = state->totalSize;
    state->head->next = nullptr;
}

bool Freelist::FreeSpace()
{
    U64 runningTotal = 0;
    State* state = (State*)memory;
    Node* node = state->head;

    while (node) 
    {
        runningTotal += node->size;
        node = node->next;
    }

    return runningTotal;
}

Node* Freelist::GetNode()
{
    State* state = (State*)memory;
    for (U64 i = 1; i < state->maxEntries; ++i) 
    {
        if (state->nodes[i].offset == INVALID_ID) 
        {
            return &state->nodes[i];
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

U64 Freelist::GetMemoryRequirement(U64 size)
{
    U64 maxEntries = (size / (sizeof(void*) * sizeof(Node)));
    return sizeof(State) + (sizeof(Node) * maxEntries);
}