#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Multithreading/ThreadSafety.hpp"
#include <cstringt.h>

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
NH_API inline Type* CopyData(Type* dst, const Type* src, U64 count)
{
	constexpr U64 size = IsVoid<Type> ? 1 : sizeof(Type);

	if (dst == src) { return dst; }

	Type* dest = dst;

	if (dst > src && dst < src + count)
	{
		dst += count - 1;
		src += count - 1;

		if constexpr (IsNonPrimitive<Type>)
		{
			while (count--) { new (dst--) Type(*src--); }

			return dest;
		}
		else
		{
			return (Type*)memmove(dst, src, count * size);
		}
	}
	else
	{
		if constexpr (IsNonPrimitive<Type>)
		{
			while (count--) { new (dst++) Type(*src++); }

			return dest;
		}
		else
		{
			return (Type*)memcpy(dst, src, count * size);
		}
	}
}

template<class Type>
NH_API inline Type* MoveData(Type* dst, Type* src, U64 count)
{
	constexpr U64 size = IsVoid<Type> ? 1 : sizeof(Type);

	if (dst == src) { return dst; }

	Type* dest = dst;

	if (dst > src && dst < src + count)
	{
		dst += count - 1;
		src += count - 1;

		if constexpr (IsNonPrimitive<Type> && IsMoveConstructible<Type>)
		{
			while (count--)
			{
				new (dst--) Type(Move(*src));
				if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { src->~Type(); }
				--src;
			}

			return dest;
		}
		else
		{
			return (Type*)memmove(dst, src, count * size);
		}
	}
	else
	{
		if constexpr (IsNonPrimitive<Type> && IsMoveConstructible<Type>)
		{
			while (count--)
			{
				new (dst++) Type(Move(*src));
				if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { src->~Type(); }
				++src;
			}

			return dest;
		}
		else
		{
			return (Type*)memcpy(dst, src, sizeof(Type) * count);
		}
	}
}

NH_API constexpr U64 Kilobytes(U64 n) { return n * 1024Ui64; }
NH_API constexpr U64 Megabytes(U64 n) { return n * 1024Ui64 * 1024Ui64; }
NH_API constexpr U64 Gigabytes(U64 n) { return n * 1024Ui64 * 1024Ui64 * 1024Ui64; }

#ifndef MEMORY_SIZE
static constexpr inline U64 DynamicMemorySize = Gigabytes(1);
#else
static constexpr inline U64 DynamicMemorySize = MEMORY_SIZE;
#endif

enum class RegionSize : U64
{
	KB1 = Kilobytes(1),
	KB16 = Kilobytes(16),
	KB256 = Kilobytes(256),
	MB4 = Megabytes(4),
};

struct Region1kb { U8 memory[*RegionSize::KB1]; };
struct Region16kb { U8 memory[*RegionSize::KB16]; };
struct Region256kb { U8 memory[*RegionSize::KB256]; };
struct Region4mb { U8 memory[*RegionSize::MB4]; };

template<class Region>
class MemoryRegion
{
	using RegionType = Region;

private:
	static void Create(void* pointer, U32 cap, U32* indices)
	{
		capacity = cap;
		region = (Region*)pointer;
		freeIndices = indices;
	}

	static bool Allocate(void** pointer)
	{
		if (Full()) { return false; }

		*pointer = region + GetFree();
		return true;
	}

	static bool Reallocate(void** src, void** dst);

	static void Free(void** pointer)
	{
		memset(*pointer, 0, sizeof(Region));

		Release((U32)((Region*)*pointer - region));
		*pointer = nullptr;
	}

	static bool WithinRegion(void* pointer)
	{
		return pointer < region + capacity;
	}

	static U32 GetFree()
	{
		U32 index = ThreadSafety::SafeDecrement32((L32*)&freeCount);

		if (index < capacity) { return freeIndices[index]; }

		++freeCount;
		return ThreadSafety::SafeIncrement32((L32*)&lastFree) - 1;
	}

	static void Release(U32 index)
	{
		freeIndices[ThreadSafety::SafeIncrement32((L32*)&freeCount) - 1] = index;
	}

	static NH_INLINE bool Full()
	{
		return lastFree >= capacity && freeCount == 0;
	}

	static U32 capacity;
	static U32 freeCount;
	static U32 lastFree;
	static U32* freeIndices;
	static Region* region;

	friend class Memory;
	friend class MemoryRegion<Region1kb>;
	friend class MemoryRegion<Region16kb>;
	friend class MemoryRegion<Region256kb>;
	friend class MemoryRegion<Region4mb>;

	STATIC_CLASS(MemoryRegion);
};

template <class Region> U32 MemoryRegion<Region>::capacity = 0;
template <class Region> U32 MemoryRegion<Region>::freeCount = 0;
template <class Region> U32 MemoryRegion<Region>::lastFree = 0;
template <class Region> U32* MemoryRegion<Region>::freeIndices = nullptr;
template <class Region> Region* MemoryRegion<Region>::region = nullptr;

class Memory
{
public:
	template<Pointer Type> static NH_API void Allocate(Type* pointer);
	template<Pointer Type> static NH_API U64 Allocate(Type* pointer, U64 count);
	template<Pointer Type> static NH_API U64 Reallocate(Type* pointer, U64 count);

	template<Pointer Type> static NH_API void Free(Type* pointer);

	static NH_API bool IsAllocated(void* pointer);

private:
	static bool Initialize();
	static void Shutdown();

	static U32 allocations;
	static U8* memory;

	static bool initialized;

	friend class Engine;

	STATIC_CLASS(Memory);
};

template<Pointer Type>
inline void Memory::Allocate(Type* pointer)
{
	static bool b = Initialize();

	if (IsAllocated(*pointer)) { return; }

	constexpr U64 size = sizeof(RemovePointer<Type>);

	if constexpr (size <= sizeof(Region1kb)) { MemoryRegion<Region1kb>::Allocate((void**)pointer); }
	else if constexpr (size <= sizeof(Region16kb)) { MemoryRegion<Region16kb>::Allocate((void**)pointer); }
	else if constexpr (size <= sizeof(Region256kb)) { MemoryRegion<Region256kb>::Allocate((void**)pointer); }
	else if constexpr (size <= sizeof(Region4mb)) { MemoryRegion<Region4mb>::Allocate((void**)pointer); }
}

template<Pointer Type>
inline U64 Memory::Allocate(Type* pointer, U64 count)
{
	static bool b = Initialize();

	if (IsAllocated(*pointer)) { return Reallocate<Type>(pointer, count); }

	U64 size = sizeof(RemovePointer<Type>) * count;

	if (size <= sizeof(Region1kb)) { MemoryRegion<Region1kb>::Allocate((void**)pointer); return sizeof(Region1kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region16kb)) { MemoryRegion<Region16kb>::Allocate((void**)pointer); return sizeof(Region16kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region256kb)) { MemoryRegion<Region256kb>::Allocate((void**)pointer); return sizeof(Region256kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region4mb)) { MemoryRegion<Region4mb>::Allocate((void**)pointer); return sizeof(Region4mb) / sizeof(RemovePointer<Type>); }

	return 0;
}

template<Pointer Type>
inline U64 Memory::Reallocate(Type* pointer, U64 count)
{
	static bool b = Initialize();

	if (!IsAllocated(*pointer)) { return Allocate<Type>(pointer, count); }
	
	U64 size = sizeof(RemovePointer<Type>) * count;

	Type temp = nullptr;

	if (size <= sizeof(Region1kb)) { MemoryRegion<Region1kb>::Reallocate((void**)pointer, (void**)&temp); return sizeof(Region1kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region16kb)) { MemoryRegion<Region16kb>::Reallocate((void**)pointer, (void**)&temp); return sizeof(Region16kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region256kb)) { MemoryRegion<Region256kb>::Reallocate((void**)pointer, (void**)&temp); return sizeof(Region256kb) / sizeof(RemovePointer<Type>); }
	else if (size <= sizeof(Region4mb)) { MemoryRegion<Region4mb>::Reallocate((void**)pointer, (void**)&temp); return sizeof(Region4mb) / sizeof(RemovePointer<Type>); }

	return 0;
}

template<Pointer Type>
inline void Memory::Free(Type* pointer)
{
	if (!IsAllocated(*pointer)) { return; }

	if (MemoryRegion<Region1kb>::WithinRegion(*pointer)) { MemoryRegion<Region1kb>::Free((void**)pointer); }
	else if (MemoryRegion<Region16kb>::WithinRegion(*pointer)) { MemoryRegion<Region16kb>::Free((void**)pointer); }
	else if (MemoryRegion<Region256kb>::WithinRegion(*pointer)) { MemoryRegion<Region256kb>::Free((void**)pointer); }
	else if (MemoryRegion<Region4mb>::WithinRegion(*pointer)) { MemoryRegion<Region4mb>::Free((void**)pointer); }
}

inline bool Memory::IsAllocated(void* pointer)
{
	return pointer != nullptr && pointer >= memory && pointer < memory + DynamicMemorySize;
}

template<class Region>
inline bool MemoryRegion<Region>::Reallocate(void** src, void** dst)
{
	Allocate(dst);

	if (MemoryRegion<Region1kb>::WithinRegion(*src))
	{
		memmove(*dst, *src, sizeof(Region1kb));
		MemoryRegion<Region1kb>::Free(src);
	}
	else if (MemoryRegion<Region16kb>::WithinRegion(*src))
	{
		memmove(*dst, *src, sizeof(Region16kb));
		MemoryRegion<Region16kb>::Free(src);
	}
	else if (MemoryRegion<Region256kb>::WithinRegion(*src))
	{
		memmove(*dst, *src, sizeof(Region256kb));
		MemoryRegion<Region256kb>::Free(src);
	}
	else if (MemoryRegion<Region4mb>::WithinRegion(*src))
	{
		memmove(*dst, *src, sizeof(Region4mb));
		MemoryRegion<Region4mb>::Free(src);
	}

	*src = *dst;

	return true;
}

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