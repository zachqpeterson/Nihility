#pragma once

#include "Defines.hpp"

#include <string.h>

#define STATIC_SIZE 1073741824
#define DYNAMIC_SIZE 1073741824

/*
* TODO: Override new and delete globally
* TODO: If one size is full, allocate next size
* TODO: Debug Memory stats
* TODO: memcpy, memset, memset(ptr, 0, size) defines
*/

/// <summary>
/// This is a general purpose memory allocator, with linear and dynamic allocating, NO garbage collection
/// </summary>
class NH_API Memory
{
	struct Region1kb { U64 unused[128]; };
	struct Region16kb { Region1kb unused[16]; };
	struct Region256kb { Region16kb unused[16]; };
	struct Region4mb { Region256kb unused[16]; };

public:
	template<typename T>
	static void Allocate(T** pointer);

	template<typename T, Unsigned Int>
	static void Allocate(T** pointer, Int& outSize);

	template<typename T, Unsigned Int>
	static void AllocateSize(T** pointer, Int size);

	template<typename T, Unsigned Int>
	static void AllocateArray(T** pointer, Int& count);

	template<typename T, Unsigned Int>
	static void AllocateArray(T** pointer, const Int& count);

	template<typename T, Unsigned Int>
	static void Reallocate(T** pointer, Int& count);

	template<typename T, Unsigned Int>
	static void Reallocate(T** pointer, const Int& count);

	template<typename T>
	static void Free(T** pointer);

	template<typename T>
	static void FreeArray(T** pointer);

	template<typename T>
	static void AllocateStatic(T** pointer);

private:
	static bool Initialize();
	static void Shutdown();

	static void Allocate1kb(void** pointer);
	static void Allocate16kb(void** pointer);
	static void Allocate256kb(void** pointer);
	static void Allocate4mb(void** pointer);

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

template<typename T> 
inline void Memory::Allocate(T** pointer)
{
	constexpr U64 size = sizeof(T);

	if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
}

template<typename T, Unsigned Int>
inline void Memory::Allocate(T** pointer, Int& outSize)
{
	constexpr U64 size = sizeof(T);

	if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); outSize = sizeof(Region1kb); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); outSize = sizeof(Region16kb); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); outSize = sizeof(Region256kb); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); outSize = sizeof(Region4mb); return; }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
}

template<typename T, Unsigned Int>
inline void Memory::AllocateSize(T** pointer, Int size)
{
	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

	BreakPoint;
}

template<typename T, Unsigned Int>
inline void Memory::AllocateArray(T** pointer, Int& count)
{
	constexpr U64 size = sizeof(T);

	if (count == 0)
	{
		if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); count = sizeof(Region1kb) / size; return; }
		else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); count = sizeof(Region16kb) / size; return; }
		else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); count = sizeof(Region256kb) / size; return; }
		else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); count = sizeof(Region4mb) / size; return; }

		BreakPoint;
		//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
	}
	else
	{
		if (size * count <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); count = sizeof(Region1kb) / size; return; }
		else if (size * count <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); count = sizeof(Region16kb) / size; return; }
		else if (size * count <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); count = sizeof(Region256kb) / size; return; }
		else if (size * count <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); count = sizeof(Region4mb) / size; return; }

		BreakPoint;
		//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
	}
}

template<typename T, Unsigned Int>
inline void Memory::AllocateArray(T** pointer, const Int& count)
{
	constexpr U64 size = sizeof(T);

	if (count == 0)
	{
		if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
		else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
		else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
		else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

		BreakPoint;
		//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
	}
	else
	{
		if (size * count <= sizeof(Region1kb)) { Allocate1kb((void**)pointer); return; }
		else if (size * count <= sizeof(Region16kb)) { Allocate16kb((void**)pointer); return; }
		else if (size * count <= sizeof(Region256kb)) { Allocate256kb((void**)pointer); return; }
		else if (size * count <= sizeof(Region4mb)) { Allocate4mb((void**)pointer); return; }

		BreakPoint;
		//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, sizeof(Region4mb));
	}
}

template<typename T, Unsigned Int>
inline void Memory::Reallocate(T** pointer, Int& count)
{
	constexpr U64 size = sizeof(T);

	void* temp = nullptr;

	if (count == 0)
	{
		if constexpr (size <= sizeof(Region1kb)) { Allocate1kb(&temp); count = sizeof(Region1kb) / size; }
		else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb(&temp); count = sizeof(Region16kb) / size; }
		else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb(&temp); count = sizeof(Region256kb) / size; }
		else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb(&temp); count = sizeof(Region4mb) / size; }
	}
	else
	{
		if (size * count <= sizeof(Region1kb)) { Allocate1kb(&temp); count = sizeof(Region1kb) / size; }
		else if (size * count <= sizeof(Region16kb)) { Allocate16kb(&temp); count = sizeof(Region16kb) / size; }
		else if (size * count <= sizeof(Region256kb)) { Allocate256kb(&temp); count = sizeof(Region256kb) / size; }
		else if (size * count <= sizeof(Region4mb)) { Allocate4mb(&temp); count = sizeof(Region4mb) / size; }
	}

	if (*pointer)
	{
		void* cmp = *pointer;

		if (cmp >= pool4mbPointer) { memcpy(temp, *pointer, sizeof(Region4mb)); Free4mb((void**)pointer); }
		else if (cmp >= pool256kbPointer) { memcpy(temp, *pointer, sizeof(Region256kb)); Free256kb((void**)pointer); }
		else if (cmp >= pool16kbPointer) { memcpy(temp, *pointer, sizeof(Region16kb)); Free16kb((void**)pointer); }
		else if (cmp >= pool1kbPointer) { memcpy(temp, *pointer, sizeof(Region1kb)); Free1kb((void**)pointer); }
	}

	*pointer = (T*)temp;
}

template<typename T, Unsigned Int>
static void Memory::Reallocate(T** pointer, const Int& count)
{
	constexpr U64 size = sizeof(T);

	void* temp = nullptr;

	if (count == 0)
	{
		if constexpr (size <= sizeof(Region1kb)) { Allocate1kb(&temp); }
		else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb(&temp); }
		else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb(&temp); }
		else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb(&temp); }
	}
	else
	{
		if (size * count <= sizeof(Region1kb)) { Allocate1kb(&temp); }
		else if (size * count <= sizeof(Region16kb)) { Allocate16kb(&temp); }
		else if (size * count <= sizeof(Region256kb)) { Allocate256kb(&temp); }
		else if (size * count <= sizeof(Region4mb)) { Allocate4mb(&temp); }
	}

	if (*pointer)
	{
		void* cmp = *pointer;

		if (cmp >= pool4mbPointer) { memcpy(temp, *pointer, sizeof(Region4mb)); Free4mb((void**)pointer); }
		else if (cmp >= pool256kbPointer) { memcpy(temp, *pointer, sizeof(Region256kb)); Free256kb((void**)pointer); }
		else if (cmp >= pool16kbPointer) { memcpy(temp, *pointer, sizeof(Region16kb)); Free16kb((void**)pointer); }
		else if (cmp >= pool1kbPointer) { memcpy(temp, *pointer, sizeof(Region1kb)); Free1kb((void**)pointer); }
	}

	*pointer = (T*)temp;
}

template<typename T> 
inline void Memory::Free(T** pointer)
{
	if (!pointer) { return; }

	constexpr U64 size = sizeof(T);

	if constexpr (size <= sizeof(Region1kb)) { Free1kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Free16kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Free256kb((void**)pointer); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Free4mb((void**)pointer); return; }

	//Logger::Error("Pointer '{}' wasn't allocated with Memory::Allocate or Memory::AllocateArray", *pointer);
}

template<typename T> 
inline void Memory::FreeArray(T** pointer)
{
	if (!pointer) { return; }

	void* cmp = *pointer;

	if (cmp >= pool4mbPointer) { Free4mb((void**)pointer); }
	else if (cmp >= pool256kbPointer) { Free256kb((void**)pointer); }
	else if (cmp >= pool16kbPointer) { Free16kb((void**)pointer); }
	else if (cmp >= pool1kbPointer) { Free1kb((void**)pointer); }

	//Logger::Error("Pointer '{}' wasn't allocated with Memory::Allocate or Memory::AllocateArray", *pointer);
}

template<typename T> 
inline void Memory::AllocateStatic(T** pointer)
{
	static bool init = Initialize();
	constexpr U64 size = sizeof(T);

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = staticPointer;
		staticPointer += size;

		return;
	}

	//Logger::Error("Not enough space to allocate size of '{}', space left: {}", size, memory + totalSize);
}