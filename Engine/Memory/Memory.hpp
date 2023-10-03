#pragma once

#include "Defines.hpp"
#include "Containers\Freelist.hpp"

#define KILOBYTES(c) c * 1024Ui64
#define MEGABYTES(c) c * 1024Ui64 * 1024Ui64
#define GIGABYTES(c) c * 1024Ui64 * 1024Ui64 * 1024Ui64

#define STATIC_SIZE 1073741824Ui64
#define DYNAMIC_SIZE 1073741824Ui64

/*
* TODO: Debug Memory stats
* TODO: Pages
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
	template<Pointer Type> static void AllocateSize(Type* pointer, const U64& size);
	template<Pointer Type, Unsigned Int> static void AllocateSize(Type* pointer, const U64& size, Int& newSize);
	template<Pointer Type> static void AllocateArray(Type* pointer, const U64& count);
	template<Pointer Type, Unsigned Int> static void AllocateArray(Type* pointer, const U64& count, Int& newCount);
	template<Pointer Type> static void Reallocate(Type* pointer, const U64& count);
	template<Pointer Type, Unsigned Int> static void Reallocate(Type* pointer, const U64& count, Int& newCount);

	template<Pointer Type> static void Free(Type* pointer);

	template<Pointer Type> static void AllocateStatic(Type* pointer);

	static bool IsDynamicallyAllocated(void* pointer);
	static bool IsStaticallyAllocated(void* pointer);

	static U64 MemoryAlign(U64 size, U64 alignment);

	static void Set(void* pointer, U8 value, U64 size);
	static void Zero(void* pointer, U64 size);
	static void Copy(void* dst, const void* src, U64 size);
	template<class T>
	static bool Compare(const T* a, const T* b, U64 length);

private:
	static bool Initialize();
	static void Shutdown();

	static void Allocate1kb(void** pointer, U64 size);
	static void Allocate16kb(void** pointer, U64 size);
	static void Allocate256kb(void** pointer, U64 size);
	static void Allocate4mb(void** pointer, U64 size);

	static void FreeChunk(void** pointer);
	static void CopyFree(void** pointer, void* copy, U64 size);

	static void Free1kb(void** ptr);
	static void Free16kb(void** ptr);
	static void Free256kb(void** ptr);
	static void Free4mb(void** ptr);

	static U8* memory;
	static U64 totalSize;

	static U8* dynamicPointer;
	static U8* staticPointer;

	static Region1kb* pool1kbPointer;
	static Freelist free1kbIndices;

	static Region16kb* pool16kbPointer;
	static Freelist free16kbIndices;

	static Region256kb* pool256kbPointer;
	static Freelist free256kbIndices;

	static Region4mb* pool4mbPointer;
	static Freelist free4mbIndices;

	static inline bool initialized{ false };

	STATIC_CLASS(Memory);
	friend class Engine;
};

template<Pointer Type>
inline void Memory::Allocate(Type* pointer)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); return; }

	*pointer = (Type)calloc(1, size);
}

template<Pointer Type>
inline void Memory::AllocateSize(Type* pointer, const U64& size)
{
	if (!initialized) { Initialize(); }

	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); return; }

	*pointer = (Type)calloc(1, size);
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateSize(Type* pointer, const U64& size, Int& newSize)
{
	if (!initialized) { Initialize(); }

	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); newSize = sizeof(Region1kb); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); newSize = sizeof(Region16kb); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); newSize = sizeof(Region256kb); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); newSize = sizeof(Region4mb); return; }

	*pointer = (Type)calloc(1, size);
	newSize = (Int)size;
}

template<Pointer Type>
inline void Memory::AllocateArray(Type* pointer, const U64& count)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);
	const U64 arraySize = size * count;

	if (arraySize <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, arraySize); return; }

	*pointer = (Type)calloc(1, arraySize);
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateArray(Type* pointer, const U64& count, Int& newCount)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);
	const U64 arraySize = size * count;

	if (arraySize <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, arraySize); newCount = sizeof(Region1kb) / size; return; }
	else if (arraySize <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, arraySize); newCount = sizeof(Region16kb) / size; return; }
	else if (arraySize <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, arraySize); newCount = sizeof(Region256kb) / size; return; }
	else if (arraySize <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, arraySize); newCount = sizeof(Region4mb) / size; return; }

	*pointer = (Type)calloc(1, arraySize);
	newCount = (Int)count;
}

template<Pointer Type>
inline void Memory::Reallocate(Type* pointer, const U64& count)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (!IsDynamicallyAllocated(*pointer) && *pointer != nullptr)
	{
		if (IsStaticallyAllocated(*pointer)) { return; }

		realloc(*pointer, size * count);
		return;
	}

	const U64 totalSize = size * count;

	Type temp = nullptr;

	if (totalSize <= sizeof(Region1kb)) { Allocate1kb((void**)&temp, totalSize); }
	else if (totalSize <= sizeof(Region16kb)) { Allocate16kb((void**)&temp, totalSize); }
	else if (totalSize <= sizeof(Region256kb)) { Allocate256kb((void**)&temp, totalSize); }
	else if (totalSize <= sizeof(Region4mb)) { Allocate4mb((void**)&temp, totalSize); }

	if (*pointer != nullptr)
	{
		CopyFree((void**)pointer, (void*)temp, count);
	}

	*pointer = (Type)temp;
}

template<Pointer Type, Unsigned Int>
inline void Memory::Reallocate(Type* pointer, const U64& count, Int& newCount)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (!IsDynamicallyAllocated(*pointer) && *pointer != nullptr)
	{
		if (IsStaticallyAllocated(*pointer)) { return; }

		realloc(*pointer, size * count);
		newCount = (Int)count;
		return;
	}

	constexpr U64 count1kb = sizeof(Region1kb) / size;
	constexpr U64 count16kb = sizeof(Region16kb) / size;
	constexpr U64 count256kb = sizeof(Region256kb) / size;
	constexpr U64 count4mb = sizeof(Region4mb) / size;

	const U64 totalSize = size * count;

	Type temp = nullptr;

	if (totalSize <= sizeof(Region1kb)) { Allocate1kb((void**)&temp, totalSize); newCount = count1kb; }
	else if (totalSize <= sizeof(Region16kb)) { Allocate16kb((void**)&temp, totalSize); newCount = count16kb; }
	else if (totalSize <= sizeof(Region256kb)) { Allocate256kb((void**)&temp, totalSize); newCount = count256kb; }
	else if (totalSize <= sizeof(Region4mb)) { Allocate4mb((void**)&temp, totalSize); newCount = count4mb; }

	if (*pointer != nullptr)
	{
		CopyFree((void**)pointer, (void*)temp, count);
	}

	*pointer = (Type)temp;
}

template<Pointer Type>
inline void Memory::Free(Type* pointer)
{
	if (!initialized && *pointer == nullptr) { return; }

	if (!IsDynamicallyAllocated(*pointer))
	{
		if (IsStaticallyAllocated(*pointer)) { return; }
		free((void*)*pointer);
		return;
	}

	FreeChunk((void**)pointer);
}

template<Pointer Type>
inline void Memory::AllocateStatic(Type* pointer)
{
	if (!initialized) { Initialize(); }

	constexpr U64 size = sizeof(RemovedPointer<Type>);

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = staticPointer;
		staticPointer += size;

		return;
	}

	BreakPoint;
}

template<class T>
inline bool Memory::Compare(const T* a, const T* b, U64 length)
{
	while (length--) { if (*a++ != *b++) { return false; } }

	return true;
}