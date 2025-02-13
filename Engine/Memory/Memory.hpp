#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Platform/ThreadSafety.hpp"

#include <cstringt.h>
#include <intrin.h>

template<Character T>
NH_API inline constexpr U64 Length(const T* str) noexcept
{
	if (!str) { return 0; }

	const T* it = str;
	while (*it) { ++it; }

	return it - str;
}

NH_API inline constexpr U64 Length(NullPointer) noexcept
{
	return 0;
}

template<class T>
NH_API inline bool Compare(const T* a, const T* b, U64 length)
{
	return memcmp(a, b, length) == 0;
}

NH_API inline void* Set(void* pointer, I32 value, U64 size)
{
	return memset(pointer, value, size);
}

NH_API inline void* Zero(void* pointer, U64 size)
{
	return memset(pointer, 0, size);
}

template<class Type, class... Parameters>
NH_API inline Type& Construct(Type* dst, Parameters&&... parameters) noexcept
{
	return *(new (dst) Type(Forward<Parameters>(parameters)...));
}

template<class Type, class... Parameters>
NH_API inline Type& Assign(Type* dst, Parameters&&... parameters) noexcept
{
	return dst->operator=(Forward<Parameters>(parameters)...);
}

template<class Type>
NH_API inline Type* Copy(Type* dst, const Type* src, U64 count)
{
	constexpr U64 size = IsVoid<Type> ? 1 : sizeof(Type);

	if (dst == src) { return dst; }

	if constexpr (IsNonPrimitive<Type>)
	{
		Type* dest = dst;

		if (dst > src && dst < src + count)
		{
			dst += count - 1;
			src += count - 1;

			while (count--) { new (dst--) Type(*src--); }
		}
		else
		{
			while (count--) { new (dst++) Type(*src++); }
		}

		return dest;
	}
	else
	{
		return (Type*)memcpy(dst, src, count * size);
	}
}

template<class Type>
NH_API inline Type* Move(Type* dst, Type* src, U64 count)
{
	constexpr U64 size = IsVoid<Type> ? 1 : sizeof(Type);

	if (dst == src) { return dst; }

	if constexpr (IsNonPrimitive<Type> && IsMoveConstructible<Type>)
	{
		Type* dest = dst;

		if (dst > src && dst < src + count)
		{
			dst += count - 1;
			src += count - 1;

			while (count--)
			{
				new (dst--) Type(Move(*src));
				if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { src->~Type(); }
				--src;
			}
		}
		else
		{
			while (count--)
			{
				new (dst++) Type(Move(*src));
				if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { src->~Type(); }
				++src;
			}
		}

		return dest;
	}
	else
	{
		return (Type*)memcpy(dst, src, sizeof(Type) * count);
	}
}

NH_API constexpr U64 Kilobytes(U64 n) { return n * 1024Ui64; }
NH_API constexpr U64 Megabytes(U64 n) { return n * 1024Ui64 * 1024Ui64; }
NH_API constexpr U64 Gigabytes(U64 n) { return n * 1024Ui64 * 1024Ui64 * 1024Ui64; }

#ifndef STATIC_MEMORY_SIZE
constexpr U64 StaticMemorySize = 1024Ui64 * 1024Ui64 * 1024Ui64;
#else
constexpr U64 StaticMemorySize = STATIC_MEMORY_SIZE;
#endif

#ifndef DYNAMIC_MEMORY_SIZE
constexpr U64 DynamicMemorySize = 1024Ui64 * 1024Ui64 * 1024Ui64;
#else
constexpr U64 DynamicMemorySize = DYNAMIC_MEMORY_SIZE;
#endif

/*
* TODO: Debug Memory stats
* TODO: Pages
*/

/*---------GLOBAL NEW/DELETE---------*/

enum class Align : U64 {};

NH_NODISCARD void* operator new(U64 size);
NH_NODISCARD void* operator new[](U64 size);
NH_NODISCARD void* operator new(U64 size, Align alignment);
NH_NODISCARD void* operator new[](U64 size, Align alignment);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, Align alignment) noexcept;
void operator delete[](void* ptr, Align alignment) noexcept;
void operator delete(void* ptr, U64 size) noexcept;
void operator delete[](void* ptr, U64 size) noexcept;
void operator delete(void* ptr, U64 size, Align alignment) noexcept;
void operator delete[](void* ptr, U64 size, Align alignment) noexcept;

enum Region
{
	REGION_NONE = 0,
	REGION_1KB = Kilobytes(1),
	REGION_16KB = Kilobytes(16),
	REGION_256KB = Kilobytes(256),
	REGION_4MB = Megabytes(4),
};

struct NH_API AllocTracker
{
	U32 GetFree()
	{
		if (lastFree >= capacity && freeCount == 0) { return U32_MAX; }

		U32 index = SafeDecrement(&freeCount);

		if (index < capacity) { return freeIndices[index]; }

		++freeCount;
		return SafeIncrement(&lastFree) - 1;
	}

	void Release(U32 index)
	{
		freeIndices[SafeIncrement(&freeCount) - 1] = index;
	}

	bool Full()
	{
		return lastFree >= capacity && freeCount == 0;
	}

	U32 capacity{ 0 };
	U32 freeCount{ 0 };
	U32* freeIndices{ nullptr };
	U32 lastFree{ 0 };
};

/// <summary>
/// This is a general purpose memory allocator, with linear and dynamic allocating, with NO garbage collection
/// </summary>
class NH_API Memory
{
struct Region1kb { private: U8 memory[REGION_1KB]; };
struct Region16kb { private: U8 memory[REGION_16KB]; };
struct Region256kb { private: U8 memory[REGION_256KB]; };
struct Region4mb { private: U8 memory[REGION_4MB]; };

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
	template<Pointer Type> static void AllocateStaticSize(Type* pointer, const U64& size);
	template<Pointer Type> static void AllocateStaticArray(Type* pointer, const U64& count);

	static bool IsDynamicallyAllocated(void* pointer);
	static bool IsStaticallyAllocated(void* pointer);

private:
	static bool Initialize();
	static void Shutdown();

	static Region GetRegion(void* pointer);
	static Region GetRegion(U64 size);

	//TODO: Maybe check if pointer is already allocated
	static void Allocate1kb(void** pointer, U64 size);
	static void Allocate16kb(void** pointer, U64 size);
	static void Allocate256kb(void** pointer, U64 size);
	static void Allocate4mb(void** pointer, U64 size);
	static void* LargeAllocate(U64 size);

	static void* LargeReallocate(void** pointer, U64 size);

	static void FreeChunk(void** pointer);
	static void CopyFree(U8** pointer, U8* copy, U64 size);

	static void Free1kb(void** pointer);
	static void Free16kb(void** pointer);
	static void Free256kb(void** pointer);
	static void Free4mb(void** pointer);
	static void LargeFree(void** pointer);

	static U32 allocations;
	static U8* memory;
	static U64 totalSize;

	static U8* dynamicPointer;
	static U8* staticPointer;

	static Region1kb* pool1kbPointer;
	static AllocTracker free1kbAllocs;

	static Region16kb* pool16kbPointer;
	static AllocTracker free16kbAllocs;

	static Region256kb* pool256kbPointer;
	static AllocTracker free256kbAllocs;

	static Region4mb* pool4mbPointer;
	static AllocTracker free4mbAllocs;

	static bool initialized;

	STATIC_CLASS(Memory);
	friend class Engine;
};

template<Pointer Type>
inline void Memory::Allocate(Type* pointer)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);

	if constexpr (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); return; }
	else if constexpr (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); return; }

	*pointer = (Type)LargeAllocate(size);
}

template<Pointer Type>
inline void Memory::AllocateSize(Type* pointer, const U64& size)
{
	static bool b = Initialize();

	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); return; }

	*pointer = (Type)LargeAllocate(size);
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateSize(Type* pointer, const U64& size, Int& newSize)
{
	static bool b = Initialize();

	if (size <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, size); newSize = sizeof(Region1kb); return; }
	else if (size <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, size); newSize = sizeof(Region16kb); return; }
	else if (size <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, size); newSize = sizeof(Region256kb); return; }
	else if (size <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, size); newSize = sizeof(Region4mb); return; }

	*pointer = (Type)LargeAllocate(size);
	newSize = (Int)size;
}

template<Pointer Type>
inline void Memory::AllocateArray(Type* pointer, const U64& count)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);
	const U64 arraySize = size * count;

	if (arraySize <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, arraySize); return; }
	else if (arraySize <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, arraySize); return; }

	*pointer = (Type)LargeAllocate(arraySize);
}

template<Pointer Type, Unsigned Int>
inline void Memory::AllocateArray(Type* pointer, const U64& count, Int& newCount)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);
	const U64 arraySize = size * count;

	if (arraySize <= sizeof(Region1kb)) { Allocate1kb((void**)pointer, arraySize); newCount = sizeof(Region1kb) / size; return; }
	else if (arraySize <= sizeof(Region16kb)) { Allocate16kb((void**)pointer, arraySize); newCount = sizeof(Region16kb) / size; return; }
	else if (arraySize <= sizeof(Region256kb)) { Allocate256kb((void**)pointer, arraySize); newCount = sizeof(Region256kb) / size; return; }
	else if (arraySize <= sizeof(Region4mb)) { Allocate4mb((void**)pointer, arraySize); newCount = sizeof(Region4mb) / size; return; }

	*pointer = (Type)LargeAllocate(arraySize);
	newCount = (Int)count;
}

template<Pointer Type>
inline void Memory::Reallocate(Type* pointer, const U64& count)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);

	if (!IsDynamicallyAllocated(*pointer) && *pointer != nullptr)
	{
		if (IsStaticallyAllocated(*pointer)) { return; }

		*pointer = (Type)LargeReallocate((void**)pointer, size * count);
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
		U64 region = (U64)GetRegion(*pointer);
		CopyFree((U8**)pointer, (U8*)temp, totalSize < region ? totalSize : region);
	}

	*pointer = (Type)temp;
}

template<Pointer Type, Unsigned Int>
inline void Memory::Reallocate(Type* pointer, const U64& count, Int& newCount)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);

	if (!IsDynamicallyAllocated(*pointer) && *pointer != nullptr)
	{
		if (IsStaticallyAllocated(*pointer)) { return; }

		*pointer = (Type)LargeReallocate((void**)pointer, size * count);
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
		U64 region = (U64)GetRegion(*pointer);
		CopyFree((U8**)pointer, (U8*)temp, totalSize < region ? totalSize : region);
	}

	*pointer = (Type)temp;
}

template<Pointer Type>
inline void Memory::Free(Type* pointer)
{
	static bool b = Initialize();

	if (*pointer == nullptr) { return; }

	if (!IsDynamicallyAllocated(*pointer))
	{
		if (IsStaticallyAllocated(*pointer)) { return; }
		LargeFree((void**)pointer);
		return;
	}

	FreeChunk((void**)pointer);
}

template<Pointer Type>
inline void Memory::AllocateStatic(Type* pointer)
{
	static bool b = Initialize();

	constexpr U64 size = sizeof(RemovePointer<Type>);

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = (Type)staticPointer;
		staticPointer += size;

		return;
	}

	BreakPoint;
}

template<Pointer Type>
inline void Memory::AllocateStaticSize(Type* pointer, const U64& size)
{
	static bool b = Initialize();

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = (Type)staticPointer;
		staticPointer += size;

		return;
	}

	BreakPoint;
}

template<Pointer Type>
inline void Memory::AllocateStaticArray(Type* pointer, const U64& count)
{
	static bool b = Initialize();

	U64 size = sizeof(RemovePointer<Type>) * count;

	if (staticPointer + size <= memory + totalSize)
	{
		*pointer = (Type)staticPointer;
		staticPointer += size;

		return;
	}

	BreakPoint;
}