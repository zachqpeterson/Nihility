module;

#include "Defines.hpp"

#include <intrin.h>
#include <string.h>

export module Memory:Utilities;

export template<Character T>
inline constexpr U64 Length(const T* str) noexcept
{
	if (!str) { return 0; }

	const T* it = str;
	while (*it) { ++it; }

	return it - str;
}

export inline constexpr U64 Length(NullPointer) noexcept
{
	return 0;
}

export template<class T>
inline bool Compare(const T* a, const T* b, U64 length)
{
	return memcmp(a, b, length) == 0;
}

export inline void* Set(void* pointer, I32 value, U64 size)
{
	return memset(pointer, value, size);
}

export inline void* Zero(void* pointer, U64 size)
{
	return memset(pointer, 0, size);
}

export template<class Type>
inline Type* Copy(Type* dst, const Type* src, U64 count)
{
	if (dst == src) { return dst; }
	
	if (dst > src && dst < src + count)
	{
		return (Type*)memmove(dst, src, count * sizeof(Type));
	}
	
	//TODO: for copies of types large than 8 bytes, we could copy 8 bytes until the size left is less than 8, then regular byte copy
	
	if constexpr (sizeof(Type) % 8 == 0)
	{
		constexpr const U64 multi = sizeof(Type) / 8;
		U64* d = (U64*)dst;
		const U64* s = (const U64*)src;
		U64 n = count * multi;
		__movsq(d, s, n);
	}
	else if constexpr (sizeof(Type) % 4 == 0)
	{
		constexpr const U64 multi = sizeof(Type) / 4;
		UL32* d = (UL32*)dst;
		const UL32* s = (const UL32*)src;
		U64 n = count * multi;
		__movsd(d, s, n);
	}
	else if constexpr (sizeof(Type) % 2 == 0)
	{
		constexpr const U64 multi = sizeof(Type) / 2;
		U16* d = (U16*)dst;
		const U16* s = (const U16*)src;
		U64 n = count * multi;
		__movsw(d, s, n);
	}
	else
	{
		constexpr const U64 multi = sizeof(Type);
		U8* d = (U8*)dst;
		const U8* s = (const U8*)src;
		U64 n = count * multi;
		__movsb(d, s, n);
	}
	
	return dst;
}