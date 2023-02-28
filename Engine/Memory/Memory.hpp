#pragma once

#include "Defines.hpp"

#include <string.h>

#define STATIC_SIZE 1073741824
#define DYNAMIC_SIZE 1073741824

/// <summary>
/// This is a general purpose memory allocator, with linear and dynamic allocating, NO garbage collection
/// </summary>
class NH_API Memory
{
	struct Region1kb { U64 unused[128]; };
	struct Region16kb { Region1kb unused[16]; };
	struct Region256kb { Region16kb unused[16]; };
	struct Region1mb { Region256kb unused[4]; };

public:
	/// <summary>
	/// Allocates a block of memory, prefer to use Allocate1kb, Allocate16kb, Allocate256kb, Allocate1mb
	/// </summary>
	/// <param name="size:">The size in bytes to allocate</param>
	/// <returns>The pointer to the allocated block of memory</returns>
	static void* Allocate(U64 size);

	/// <summary>
	/// Allocates a block of memory and gives the actual size of the allocation, prefer to use Allocate1kb, Allocate16kb, Allocate256kb, Allocate1mb
	/// </summary>
	/// <param name="size:">The size in bytes to allocate</param>
	/// <param name="outSize:">The actual size allocated</param>
	/// <returns>The pointer to the allocated block of memory</returns>
	static void* Allocate(U64 size, U64& outSize);

	/// <summary>
	/// Frees a block of memory, prefer to use Free1kb, Free16kb, Free256kb, Free1mb
	/// </summary>
	/// <param name="ptr:">The pointer to the allocated block of memory to free</param>
	static void Free(void* ptr);

	/// <summary>
	/// Allocates a block of memory of size sizeof(T)
	/// </summary>
	/// <typeparam name="T">The type to allocate</typeparam>
	/// <returns>The pointer to the allocated block of memory</returns>
	template<typename T> static T* Allocate();

	/// <summary>
	/// Allocates a block of memory of size sizeof(T), gives the actual size of the allocation
	/// </summary>
	/// <param name="outSize:">The size variable to set</param>
	/// <typeparam name="T">The type to allocate</typeparam>
	/// <returns>The pointer to the allocated block of memory</returns>
	template<typename T> static T* Allocate(U64& outSize);

	/// <summary>
	/// Frees a block of memory of size sizeof(T)
	/// </summary>
	/// <typeparam name="T">The type to free</typeparam>
	/// <param name="ptr:">The pointer to the allocated block of memory to free</param>
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
	/// Frees memory and allocates a new block of memory, copying over the data
	/// </summary>
	/// <param name="ptr:">The pointer to the block of memory to free</param>
	/// <param name="size:">The size in bytes to allocate</param>
	/// <returns>The new pointer to the block of memory</returns>
	static void* Reallocate(void* ptr, U64 size);

	/// <summary>
	/// Frees the pointer and allocate a new block of memory for it, copying over the data
	/// </summary>
	/// <param name="ptr:">The pointer to the block of memory to free</param>
	/// <param name="size:">The size in bytes to allocate</param>
	/// <param name="outSize:">The actual size allocated</param>
	/// <returns>The new pointer to the block of memory</returns>
	static void* Reallocate(void* ptr, U64 size, U64& outSize);



	/// <summary>
	/// Free a 1kb block of memory
	/// </summary>
	/// <param name="ptr:">The pointer to the block of memory to free</param>
	static void Free1kb(void* ptr);

	/// <summary>
	/// Free a 16kb block of memory
	/// </summary>
	/// <param name="ptr::">The pointer to the block of memory to free</param>
	static void Free16kb(void* ptr);

	/// <summary>
	/// Free a 256kb block of memory
	/// </summary>
	/// <param name="ptr::">The pointer to the block of memory to free</param>
	static void Free256kb(void* ptr);

	/// <summary>
	/// Free a 1mb block of memory
	/// </summary>
	/// <param name="ptr::">The pointer to the block of memory to free</param>
	static void Free1mb(void* ptr);



	/// <summary>
	/// Allocates a block of memory, this memory will only be freed at the end of the application
	/// </summary>
	/// <param name="size:">The size in bytes to allocate</param>
	/// <returns>The pointer to the block of memory</returns>
	static void* AllocateStatic(U64 size);

	/// <summary>
	/// Allocates a block of memory of size sizeof(T), this memory will only be freed at the end of the application
	/// </summary>
	/// <typeparam name="T">The type to allocate</typeparam>
	/// <returns>The pointer to the block of memory</returns>
	template<typename T> static T* AllocateStatic();

private:
	static bool Initialize();
	static void Shutdown();

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

	static Region1mb* pool1mbPointer;
	static U32* free1mbIndices;
	static I64 last1mbFree;

	static bool initialized;

	STATIC_CLASS(Memory);
	friend class Engine;
};

template<typename T> inline T* Memory::Allocate()
{
	constexpr U64 size = sizeof(T);

	if constexpr(size <= 1024) { return (T*)Allocate1kb(); }
	else if constexpr (size <= 16384) { return (T*)Allocate16kb(); }
	else if constexpr (size <= 262144) { return (T*)Allocate256kb(); }
	else if constexpr (size <= 1048576) { return (T*)Allocate1mb(); }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, 1048576);

	return nullptr;
}

template<typename T> inline T* Memory::Allocate(U64& outSize)
{
	constexpr U64 size = sizeof(T);

	if constexpr (size <= 1024) { outSize = 1024; return (T*)Allocate1kb(); }
	else if constexpr (size <= 16384) { outSize = 16384; return (T*)Allocate16kb(); }
	else if constexpr (size <= 262144) { outSize = 262144; return (T*)Allocate256kb(); }
	else if constexpr (size <= 1048576) { outSize = 1048576; return (T*)Allocate1mb(); }

	BreakPoint;
	//Logger::Error("Allocation size '{}' too big, maximum is '{}'", size, 1048576);

	return nullptr;
}

template<typename T> inline void Memory::Free(T* ptr)
{
	if (ptr >= (T*)pool1mbPointer) { Free1mb(ptr); }
	else if (ptr >= (T*)pool256kbPointer) { Free256kb(ptr); }
	else if (ptr >= (T*)pool16kbPointer) { Free16kb(ptr); }
	else if (ptr >= (T*)pool1kbPointer) { Free1kb(ptr); }
	else
	{
		BreakPoint;
		//TODO: error, too large
		//TODO: check if pointer is past pool1mb range
	}
}

template<typename T> inline T* Memory::AllocateStatic()
{
	static bool init = Initialize();
	constexpr U64 size = sizeof(T);

	if (staticPointer + size <= memory + totalSize)
	{
		U8* block = staticPointer;
		staticPointer += size;

		return (T*)block;
	}

	return nullptr;
}