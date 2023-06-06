#pragma once

#include "Defines.hpp"
#include "Platform\ThreadSafety.hpp"

#define STATIC_SIZE 1073741824
#define DYNAMIC_SIZE 1073741824

/*
* TODO: If one size is full, allocate next size
* TODO: Debug Memory stats
*/

/*---------GLOBAL NEW/DELETE---------*/

NH_NODISCARD void* operator new (U64 size);
NH_NODISCARD void* operator new[](U64 size);
void operator delete (void* ptr);
void operator delete[](void* ptr);

/// <summary>
/// This is a general purpose memory allocator, with linear and dynamic allocating, with NO garbage collection
/// </summary>
class NH_API Memory
{
struct Region1kb { private: U64 unused[128]; };
struct Region16kb { private: Region1kb unused[16]; };
struct Region256kb { private: Region16kb unused[16]; };
struct Region4mb { private: Region256kb unused[16]; };

public:
	template<Pointer Type> static void Allocate(Type* pointer);
	template<Pointer Type, Unsigned Int> static void AllocateSize(Type* pointer, const Int& size);
	template<Pointer Type, Unsigned Int> static void AllocateSize(Type* pointer, const Int& size, Int& newSize);
	template<Pointer Type, Unsigned Int> static void AllocateArray(Type* pointer, const Int& count);
	template<Pointer Type, Unsigned Int> static void AllocateArray(Type* pointer, const Int& count, Int& newCount);
	template<Pointer Type, Unsigned Int> static void Reallocate(Type* pointer, const Int& count);
	template<Pointer Type, Unsigned Int> static void Reallocate(Type* pointer, const Int& count, Int& newCount);

	template<Pointer Type> static void Free(Type* pointer);
	template<Pointer Type> static void FreeSize(Type* pointer);
	template<Pointer Type> static void FreeArray(Type* pointer);

	template<Pointer Type> static void AllocateStatic(Type* pointer);

	static bool IsAllocated(void* pointer);

	static U64 MemoryAlign(U64 size, U64 alignment);

	static void Set(void* pointer, U8 value, U64 size);
	static void Zero(void* pointer, U64 size);
	static void Copy(void* dst, const void* src, U64 size);

private:
	static bool Initialize();
	static void Shutdown();

	static void Allocate1kb(void** pointer);
	static void Allocate16kb(void** pointer);
	static void Allocate256kb(void** pointer);
	static void Allocate4mb(void** pointer);

	static void Free(void** ptr);
	static void CopyFree(void** ptr, void* copy, U64 size);

	static void Free1kb(void** ptr);
	static void Free16kb(void** ptr);
	static void Free256kb(void** ptr);
	static void Free4mb(void** ptr);

	static U8* memory;
	static U64 totalSize;

	static U64 staticSize;
	static U8* staticPointer;

	static Region1kb* pool1kbPointer;
	static U32* free1kbIndices;
	static I64 last1kbFree;

	static Region16kb* pool16kbPointer;
	static U32* free16kbIndices;
	static I64 last16kbFree;

	static Region256kb* pool256kbPointer;
	static U32* free256kbIndices;
	static I64 last256kbFree;

	static Region4mb* pool4mbPointer;
	static U32* free4mbIndices;
	static I64 last4mbFree;

	static bool initialized;

	STATIC_CLASS(Memory);
	friend class Engine;
};

template<Pointer Type>
inline void Memory::Allocate(Type* pointer)
{
	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

	BreakPoint;
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateSize(Type* pointer, const Int& size)
{
	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

	BreakPoint;
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateSize(Type* pointer, const Int& size, Int& newSize)
{
	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); newSize = sizeof(Region1kb); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); newSize = sizeof(Region16kb); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); newSize = sizeof(Region256kb); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); newSize = sizeof(Region4mb); return; }

	BreakPoint;
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateArray(Type* pointer, const Int& count)
{
	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (size * count <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
	else if (size * count <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
	else if (size * count <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
	else if (size * count <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

	BreakPoint;
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateArray(Type* pointer, const Int& count, Int& newCount)
{
	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (size * count <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); newCount = sizeof(Region1kb) / size; return; }
	else if (size * count <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); newCount = sizeof(Region16kb) / size; return; }
	else if (size * count <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); newCount = sizeof(Region256kb) / size; return; }
	else if (size * count <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); newCount = sizeof(Region4mb) / size; return; }

	BreakPoint;
}

template<Pointer Type, Unsigned Int>
static void Memory::Reallocate(Type* pointer, const Int& count)
{
	constexpr U64 size = sizeof(RemovedPointer<Type>);
	const U64 totalSize = size * count;

	Type temp = nullptr;

	if (totalSize <= sizeof(Region1kb)) { Allocate1kb((void**)&temp); }
	else if (totalSize <= sizeof(Region16kb)) { Allocate16kb((void**)&temp); }
	else if (totalSize <= sizeof(Region256kb)) { Allocate256kb((void**)&temp); }
	else if (totalSize <= sizeof(Region4mb)) { Allocate4mb((void**)&temp); }

	if (*pointer != nullptr)
	{
		CopyFree((void**)pointer, (void*)temp, count);
	}

	*pointer = (Type)temp;
}

template<Pointer Type, Unsigned Int>
static void Memory::Reallocate(Type* pointer, const Int& count, Int& newCount)
{
	constexpr U64 size = sizeof(RemovedPointer<Type>);
	constexpr U64 count1kb = sizeof(Region1kb) / size;
	constexpr U64 count16kb = sizeof(Region16kb) / size;
	constexpr U64 count256kb = sizeof(Region256kb) / size;
	constexpr U64 count4mb = sizeof(Region4mb) / size;

	const U64 totalSize = size * count;

	Type temp = nullptr;

	if (totalSize <= sizeof(Region1kb)) { Allocate1kb((void**)&temp); newCount = count1kb; }
	else if (totalSize <= sizeof(Region16kb)) { Allocate16kb((void**)&temp); newCount = count16kb; }
	else if (totalSize <= sizeof(Region256kb)) { Allocate256kb((void**)&temp); newCount = count256kb; }
	else if (totalSize <= sizeof(Region4mb)) { Allocate4mb((void**)&temp); newCount = count4mb; }

	if (*pointer != nullptr)
	{
		CopyFree((void**)pointer, (void*)temp, count);
	}

	*pointer = (Type)temp;
}

template<Pointer Type>
inline void Memory::Free(Type* pointer)
{
	if (!IsAllocated(*pointer)) { return; }

	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if constexpr (size <= sizeof(Region1kb)) { Free1kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Free16kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Free256kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Free4mb((void**)pointer); return; }

	BreakPoint;
}

template<Pointer Type>
inline void Memory::FreeSize(Type* pointer)
{
	if (!IsAllocated(*pointer)) { return; }

	void* cmp = *pointer;

	Free((void**)pointer);
}

template<Pointer Type>
inline void Memory::FreeArray(Type* pointer)
{
	if (!IsAllocated(*pointer)) { return; }

	Free((void**)pointer);
}

template<Pointer Type>
inline void Memory::AllocateStatic(Type* pointer)
{
	static bool init = Initialize();
	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = staticPointer;
		staticPointer += size;

		return;
	}

	BreakPoint;
}