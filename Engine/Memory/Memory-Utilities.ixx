module;

#include "Defines.hpp"

#include <intrin.h>
#include <string.h>
#include <new>

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

export template<class Type, class... Parameters>
inline Type& Construct(Type* dst, Parameters&&... parameters) noexcept
{
	return *(new (dst) Type(Forward<Parameters>(parameters)...));
}

export template<class Type, class... Parameters>
inline Type& Assign(Type* dst, Parameters&&... parameters) noexcept
{
	return dst->operator=(Forward<Parameters>(parameters)...);
}

export template<class Type>
inline Type* Copy(Type* dst, const Type* src, U64 count)
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

export template<class Type>
inline Type* Move(Type* dst, Type* src, U64 count)
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