#include "Memory.hpp"

#include "Core/Logger.hpp"

void MemoryRegion::Create(U8* pointer, U32 regionSize, U32 cap, U32* indices)
{
	capacity = cap;
	region = pointer;
	freeIndices = indices;
	freeIndices = indices;
	this->regionSize = regionSize;
}

bool MemoryRegion::Allocate(void** pointer)
{
	if (Full()) { return false; }

	U32 index = GetFree();
	*pointer = region + index * regionSize;
	return true;
}

bool MemoryRegion::Reallocate(void** src, void** dst)
{
	if (Memory::region1kb.WithinRegion(*src))
	{
		if (this == &Memory::region1kb) { return false; }

		Allocate(dst);
		memmove(*dst, *src, sizeof(Region1kb));
		Memory::region1kb.Free(src);
	}
	else if (Memory::region16kb.WithinRegion(*src))
	{
		if (this == &Memory::region16kb) { return false; }

		Allocate(dst);
		memmove(*dst, *src, sizeof(Region16kb));
		Memory::region16kb.Free(src);
	}
	else if (Memory::region256kb.WithinRegion(*src))
	{
		if (this == &Memory::region256kb) { return false; }

		Allocate(dst);
		memmove(*dst, *src, sizeof(Region256kb));
		Memory::region256kb.Free(src);
	}
	else if (Memory::region4mb.WithinRegion(*src))
	{
		if (this == &Memory::region4mb) { return false; }

		Allocate(dst);
		memmove(*dst, *src, sizeof(Region4mb));
		Memory::region4mb.Free(src);
	}

	*src = *dst;

	return true;
}

void MemoryRegion::Free(void** pointer)
{
	memset(*pointer, 0, regionSize);

	U32 index = (U32)(((U8*)*pointer - region) / regionSize);
	Release(index);
	*pointer = nullptr;
}

bool MemoryRegion::WithinRegion(void* pointer)
{
	return pointer > region && pointer < region + capacity * regionSize;
}

U32 MemoryRegion::GetFree()
{
	U32 index = ThreadSafety::SafeDecrement32((L32*)&freeCount);

	if (index < capacity) { return freeIndices[index]; }

	++freeCount;
	return ThreadSafety::SafeIncrement32((L32*)&lastFree) - 1;
}

void MemoryRegion::Release(U32 index)
{
	freeIndices[ThreadSafety::SafeIncrement32((L32*)&freeCount) - 1] = index;
}

bool MemoryRegion::Full()
{
	return lastFree >= capacity && freeCount == 0;
}

U32 Memory::allocations = 0;
U8* Memory::memory = nullptr;

MemoryRegion Memory::region1kb;
MemoryRegion Memory::region16kb;
MemoryRegion Memory::region256kb;
MemoryRegion Memory::region4mb;

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

		memory = (U8*)_aligned_malloc(DynamicMemorySize + freeListMemory, 32);
		if (!memory) { return initialized = false; }

		memset(memory, 0, DynamicMemorySize + freeListMemory);

		U32* freeLists = (U32*)(memory + DynamicMemorySize);

		region1kb.Create(memory, sizeof(Region1kb), region1kbCount, freeLists);
		region16kb.Create(region1kb.region + region1kbCount * sizeof(Region1kb), sizeof(Region16kb), region16kbCount, region1kb.freeIndices + region1kb.capacity);
		region256kb.Create(region16kb.region + region16kbCount * sizeof(Region16kb), sizeof(Region256kb), region256kbCount, region16kb.freeIndices + region16kb.capacity);
		region4mb.Create(region256kb.region + region256kbCount * sizeof(Region256kb), sizeof(Region4mb), region4mbCount, region256kb.freeIndices + region256kb.capacity);
	}

	return true;
}

void Memory::Shutdown()
{
	Logger::Trace("Cleaning Up Memory...");

	initialized = false;
}

U64 Memory::AllocateInternal(void** pointer, U64 size, U64 typeSize)
{
	if (size <= sizeof(Region1kb)) { region1kb.Allocate((void**)pointer); return sizeof(Region1kb) / typeSize; }
	else if (size <= sizeof(Region16kb)) { region16kb.Allocate((void**)pointer); return sizeof(Region16kb) / typeSize; }
	else if (size <= sizeof(Region256kb)) { region256kb.Allocate((void**)pointer); return sizeof(Region256kb) / typeSize; }
	else if (size <= sizeof(Region4mb)) { region4mb.Allocate((void**)pointer); return sizeof(Region4mb) / typeSize; }
	else { *pointer = malloc(size); return size / typeSize; }

	return 0;
}

U64 Memory::ReallocateInternal(void** src, void** dst, U64 size, U64 typeSize)
{
	if (size <= sizeof(Region1kb)) { region1kb.Reallocate(src, dst); return sizeof(Region1kb) / typeSize; }
	else if (size <= sizeof(Region16kb)) { region16kb.Reallocate(src, dst); return sizeof(Region16kb) / typeSize; }
	else if (size <= sizeof(Region256kb)) { region256kb.Reallocate(src, dst); return sizeof(Region256kb) / typeSize; }
	else if (size <= sizeof(Region4mb)) { region4mb.Reallocate(src, dst); return sizeof(Region4mb) / typeSize; }
	else { *dst = realloc(*src, size); return size / typeSize; }

	return 0;
}

void Memory::FreeInternal(void** pointer)
{
	if (!IsAllocated(*pointer)) { free(*pointer); return; }

	if (region1kb.WithinRegion(*pointer)) { region1kb.Free(pointer); }
	else if (region16kb.WithinRegion(*pointer)) { region16kb.Free(pointer); }
	else if (region256kb.WithinRegion(*pointer)) { region256kb.Free(pointer); }
	else if (region4mb.WithinRegion(*pointer)) { region4mb.Free(pointer); }
}

bool Memory::IsAllocated(void* pointer)
{
	return pointer != nullptr && pointer >= memory && pointer < memory + DynamicMemorySize;

	new int();
}

NH_NODISCARD __declspec(allocator) void* operator new(U64 size) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD __declspec(allocator) void* operator new[](U64 size) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD __declspec(allocator) void* operator new(U64 size, Align alignment) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
NH_NODISCARD __declspec(allocator) void* operator new[](U64 size, Align alignment) { if (size == 0) { return nullptr; } U8* ptr; Memory::Allocate(&ptr, size); return ptr; }
void operator delete(void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size) noexcept { Memory::Free(&ptr); }
void operator delete(void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }
void operator delete[](void* ptr, U64 size, Align alignment) noexcept { Memory::Free(&ptr); }