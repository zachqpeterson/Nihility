#include "Memory.hpp"

U32 Memory::allocations = 0;
U8* Memory::memory = nullptr;

bool Memory::initialized = false;

bool Memory::Initialize()
{
	if (!ThreadSafety::SafeCheckAndSet32((volatile L32*)&initialized, 0))
	{
		U64 maxKilobytes = DynamicMemorySize / 1024;

		U32 region4mbCount = U32(maxKilobytes / 81920);
		U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
		U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
		U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region4mbCount * 4096));

		U32 freeListMemory = (region4mbCount + region256kbCount + region16kbCount + region1kbCount) * sizeof(U32);

		memory = (U8*)calloc(1, DynamicMemorySize + freeListMemory);

		if (!memory) { return initialized = false; }

		MemoryRegion<Region1kb>::region = (Region1kb*)(memory);
		MemoryRegion<Region16kb>::region = (Region16kb*)(memory + region1kbCount);
		MemoryRegion<Region256kb>::region = (Region256kb*)(memory + region16kbCount);
		MemoryRegion<Region4mb>::region = (Region4mb*)(memory + region256kbCount);

		U32* freeLists = (U32*)(memory + DynamicMemorySize);

		MemoryRegion<Region1kb>::capacity = region1kbCount;
		MemoryRegion<Region1kb>::freeIndices = freeLists;

		MemoryRegion<Region16kb>::capacity = region16kbCount;
		MemoryRegion<Region16kb>::freeIndices = MemoryRegion<Region1kb>::freeIndices + MemoryRegion<Region1kb>::capacity;

		MemoryRegion<Region256kb>::capacity = region256kbCount;
		MemoryRegion<Region256kb>::freeIndices = MemoryRegion<Region16kb>::freeIndices + MemoryRegion<Region16kb>::capacity;

		MemoryRegion<Region4mb>::capacity = region4mbCount;
		MemoryRegion<Region4mb>::freeIndices = MemoryRegion<Region256kb>::freeIndices + MemoryRegion<Region256kb>::capacity;
	}

	return true;
}

void Memory::Shutdown()
{
	initialized = false;
}

NH_NODISCARD void* operator new(U64 size) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD void* operator new[](U64 size) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD void* operator new(U64 size, Align alignment) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD void* operator new[](U64 size, Align alignment) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
void operator delete(void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }