#pragma once

#include "Defines.hpp"

#include <string.h>

/// <summary>
/// This is a general purpose memory allocator, with linear and dynamic allocating, NO garbage collection
/// </summary>
class NH_API Memory
{
	struct Region1kb { U64 a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };
	struct Region16kb { Region1kb a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };
	struct Region256kb { Region16kb a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };
	struct Region1mb { Region256kb a, b, c, d; };

public:
	/// <summary>
	/// Allocates a block of memory, prefer to use Allocate1kb, Allocate16kb, Allocate256kb, Allocate1mb
	/// </summary>
	/// <param name="size">Size size in bytes to allocate</param>
	/// <returns>The pointer to the allocated block of memory</returns>
	static void* Allocate(U64 size);

	/// <summary>
	/// Frees a block of memory, prefer to use Free1kb, Free16kb, Free256kb, Free1mb
	/// </summary>
	/// <param name="ptr">The pointer to the allocated block of memory to free</param>
	static void Free(void* ptr);

	/// <summary>
	/// Allocates a block of memory of size sizeof(T)
	/// </summary>
	/// <typeparam name="T">The type to allocate</typeparam>
	/// <returns>The pointer to the allocated block of memory</returns>
	template<typename T> static T* Allocate();

	/// <summary>
	/// Frees a block of memory of size sizeof(T)
	/// </summary>
	/// <typeparam name="T">The type to free</typeparam>
	/// <param name="ptr">The pointer to the allocated block of memory to free</param>
	template<typename T> static void Free(T* ptr);



	/// <summary>
	/// Allocates a 1kb block of memory
	/// </summary>
	/// <returns>The pointer to the block of memory</returns>
	static void* Allocate1kb();

	/// <summary>
	/// Allocates a 16kb block of memory
	/// </summary>
	/// <returns>The pointer to the block of memory</returns>
	static void* Allocate16kb();

	/// <summary>
	/// Allocates a 256kb block of memory
	/// </summary>
	/// <returns>The pointer to the block of memory</returns>
	static void* Allocate256kb();

	/// <summary>
	/// Allocates a 1mb block of memory
	/// </summary>
	/// <returns>The pointer to the block of memory</returns>
	static void* Allocate1mb();



	/// <summary>
	/// Free a 1kb block of memory
	/// </summary>
	/// <param name="ptr">The pointer to the block of memory to free</param>
	static void Free1kb(void* ptr);

	/// <summary>
	/// Free a 16kb block of memory
	/// </summary>
	/// <param name="ptr">The pointer to the block of memory to free</param>
	static void Free16kb(void* ptr);

	/// <summary>
	/// Free a 256kb block of memory
	/// </summary>
	/// <param name="ptr">The pointer to the block of memory to free</param>
	static void Free256kb(void* ptr);

	/// <summary>
	/// Free a 1mb block of memory
	/// </summary>
	/// <param name="ptr">The pointer to the block of memory to free</param>
	static void Free1mb(void* ptr);



	/// <summary>
	/// Allocates a block of memory, this memory will only be freed at the end of the application
	/// </summary>
	/// <param name="size">The size in bytes to allocate</param>
	/// <returns>The pointer to the block of memory</returns>
	static void* AllocateStatic(U64 size);

	/// <summary>
	/// Allocates a block of memory of size sizeof(T), this memory will only be freed at the end of the application
	/// </summary>
	/// <typeparam name="T">The type to allocate</typeparam>
	/// <returns>The pointer to the block of memory</returns>
	template<typename T> static T* AllocateStatic();

private:
	static bool Initialize(U64 staticSize, U64 dynamicSize);
	static void Shutdown();

	static inline U8* memory;
	static inline U64 totalSize;

	static inline U64 staticSize;
	static inline U8* staticPointer;

	static inline Region1kb* pool1kbPointer;
	static inline U32 last1kbPointer;
	static inline U32 last1kbFree;
	static inline U32* free1kbIndices;

	static inline Region16kb* pool16kbPointer;
	static inline U32 last16kbPointer;
	static inline U32 last16kbFree;
	static inline U32* free16kbIndices;

	static inline Region256kb* pool256kbPointer;
	static inline U32 last256kbPointer;
	static inline U32 last256kbFree;
	static inline U32* free256kbIndices;

	static inline Region1mb* pool1mbPointer;
	static inline U32 last1mbPointer;
	static inline U32 last1mbFree;
	static inline U32* free1mbIndices;

	STATIC_CLASS(Memory);
	friend class Engine;
};

inline bool Memory::Initialize(U64 staticBytes, U64 dynamicBytes)
{
	U64 maxKilobytes = dynamicBytes / 1024;

	U32 region1mbCount = U32(maxKilobytes / 20480);
	U32 region256kbCount = U32(maxKilobytes * 0.15f) / 256;
	U32 region16kbCount = U32(maxKilobytes * 0.3f) / 16;
	U32 region1kbCount = U32(maxKilobytes - (region16kbCount * 16) - (region256kbCount * 256) - (region1mbCount * 1024));

	U64 pointerToDynamic = sizeof(U32) * (region1kbCount + region16kbCount + region256kbCount + region1mbCount);

	totalSize = pointerToDynamic + dynamicBytes + staticBytes;

	memory = (U8*)calloc(1, totalSize);

	if (!memory) { return false; }

	free1kbIndices = (U32*)memory;
	free16kbIndices = free1kbIndices + region1kbCount;
	free256kbIndices = free16kbIndices + region16kbCount;
	free1mbIndices = free256kbIndices + region256kbCount;

	pool1kbPointer = (Region1kb*)(free1mbIndices + region1mbCount);
	last1kbPointer = 0;
	last1kbFree = 0;

	pool16kbPointer = (Region16kb*)(pool1kbPointer + region1kbCount);
	last16kbPointer = 0;
	last16kbFree = 0;

	pool256kbPointer = (Region256kb*)(pool16kbPointer + region16kbCount);
	last256kbPointer = 0;
	last256kbFree = 0;

	pool1mbPointer = (Region1mb*)(pool256kbPointer + region256kbCount);
	last1mbPointer = 0;
	last1mbFree = 0;

	staticPointer = (U8*)(pool1mbPointer + region1mbCount);
	staticSize = memory - staticPointer;

	return true;
}

inline void Memory::Shutdown()
{
	free(memory);
}

inline void* Memory::Allocate(U64 size)
{
	if (size <= 1024)
	{
		if (last1kbFree) { return pool1kbPointer + free1kbIndices[last1kbFree-- - 1]; }
		else { return pool1kbPointer + last1kbPointer++; }
	}
	else if (size <= 16384)
	{
		if (last16kbFree) { return pool16kbPointer + free16kbIndices[last16kbFree-- - 1]; }
		else { return pool16kbPointer + last16kbPointer++; }
	}
	else if (size <= 262144)
	{
		if (last256kbFree) { return pool256kbPointer + free256kbIndices[last256kbFree-- - 1]; }
		else { return pool256kbPointer + last256kbPointer++; }
	}
	else if (size <= 1048576)
	{
		if (last1mbFree) { return pool1mbPointer + free1mbIndices[last1mbFree-- - 1]; }
		else { return pool1mbPointer + last1mbPointer++; }
	}
	
	BreakPoint;
	//TODO: Log

	return nullptr;
}

inline void Memory::Free(void* ptr)
{
	if (ptr >= pool1mbPointer)
	{
		memset(ptr, 0, sizeof(Region1mb));
		free1mbIndices[last1mbFree++] = U32((Region1mb*)ptr - pool1mbPointer);
	}
	else if(ptr >= pool256kbPointer)
	{
		memset(ptr, 0, sizeof(Region256kb));
		free256kbIndices[last256kbFree++] = U32((Region256kb*)ptr - pool256kbPointer);
	}
	else if (ptr >= pool16kbPointer)
	{
		memset(ptr, 0, sizeof(Region16kb));
		free16kbIndices[last16kbFree++] = U32((Region16kb*)ptr - pool16kbPointer);
	}
	else if (ptr >= pool1kbPointer)
	{
		memset(ptr, 0, sizeof(Region1kb));
		free1kbIndices[last1kbFree++] = U32((Region1kb*)ptr - pool1kbPointer);
	}
	else
	{
		BreakPoint;
		//TODO: error, too large
		//TODO: check if pointer is past pool1mb range
	}
}

template<typename T> inline T* Memory::Allocate()
{
	constexpr U64 size = sizeof(T);

	if (size <= 1024)
	{
		if (last1kbFree) { return pool1kbPointer + free1kbIndices[last1kbFree-- - 1]; }
		else { return pool1kbPointer + last1kbPointer++; }
	}
	else if (size <= 16384)
	{
		if (last16kbFree) { return pool16kbPointer + free16kbIndices[last16kbFree-- - 1]; }
		else { return pool16kbPointer + last16kbPointer++; }
	}
	else if (size <= 262144)
	{
		if (last256kbFree) { return pool256kbPointer + free256kbIndices[last256kbFree-- - 1]; }
		else { return pool256kbPointer + last256kbPointer++; }
	}
	else if (size <= 1048576)
	{
		if (last1mbFree) { return pool1mbPointer + free1mbIndices[last1mbFree-- - 1]; }
		else { return pool1mbPointer + last1mbPointer++; }
	}
	else
	{
		BreakPoint;
		//TODO: error, too large
	}
}

template<typename T> inline void Memory::Free(T* ptr)
{
	memset(ptr, 0, sizeof(T));

	if (ptr >= pool1mbPointer)
	{
		free1mbIndices[last1mbFree++] = (Region1mb*)ptr - pool1mbPointer;
	}
	else if (ptr >= pool256kbPointer)
	{
		free256kbIndices[last256kbFree++] = (Region256kb*)ptr - pool256kbPointer;
	}
	else if (ptr >= pool16kbPointer)
	{
		free16kbIndices[last16kbFree++] = (Region16kb*)ptr - pool16kbPointer;
	}
	else if (ptr >= pool1kbPointer)
	{
		free1kbIndices[last1kbFree++] = (Region1kb*)ptr - pool1kbPointer;
	}
	else
	{
		BreakPoint;
		//TODO: error, too large
		//TODO: check if pointer is past pool1mb range
	}
}

inline void* Memory::Allocate1kb()
{
	if (last1kbFree) { return pool1kbPointer + free1kbIndices[last1kbFree-- - 1]; }
	else { return pool1kbPointer + last1kbPointer++; }
}

inline void* Memory::Allocate16kb()
{
	if (last16kbFree) { return pool16kbPointer + free16kbIndices[last16kbFree-- - 1]; }
	else { return pool16kbPointer + last16kbPointer++; }
}

inline void* Memory::Allocate256kb()
{
	if (last256kbFree) { return pool256kbPointer + free256kbIndices[last256kbFree-- - 1]; }
	else { return pool256kbPointer + last256kbPointer++; }
}

inline void* Memory::Allocate1mb()
{
	if (last1mbFree) { return pool1mbPointer + free1mbIndices[last1mbFree-- - 1]; }
	else { return pool1mbPointer + last1mbPointer++; }
}

//TODO: Debug checking that ensures this is the right size block to free
inline void Memory::Free1kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region1kb));
	free1kbIndices[last1kbFree++] = U32((Region1kb*)ptr - pool1kbPointer);
}

inline void Memory::Free16kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region16kb));
	free16kbIndices[last16kbFree++] = U32((Region16kb*)ptr - pool16kbPointer);
}

inline void Memory::Free256kb(void* ptr)
{
	memset(ptr, 0, sizeof(Region256kb));
	free256kbIndices[last256kbFree++] = U32((Region256kb*)ptr - pool256kbPointer);
}

inline void Memory::Free1mb(void* ptr)
{
	memset(ptr, 0, sizeof(Region1mb));
	free1mbIndices[last1mbFree++] = U32((Region1mb*)ptr - pool1mbPointer);
}

inline void* Memory::AllocateStatic(U64 size)
{
	if (staticPointer + size <= memory + totalSize)
	{
		U8* block = staticPointer;
		staticPointer += size;

		return block;
	}

	return nullptr;
}

template<typename T> inline T* Memory::AllocateStatic()
{
	constexpr U64 size = sizeof(T);

	if (staticPointer + size <= memory + totalSize)
	{
		U8* block = staticPointer;
		staticPointer += size;

		return block;
	}

	return nullptr;
}