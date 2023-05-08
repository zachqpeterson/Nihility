// Based on wyhash - https://github.com/wangyi-fudan/wyhash

#pragma once

#include "Defines.hpp"

#include <stdint.h>
#include <string.h>
#ifdef _MSC_VER
#pragma intrinsic(_umul128)
#endif

namespace detail
{
	NH_HEADER_STATIC constexpr U64 secret0 = 0xa0761d6478bd642full;
	NH_HEADER_STATIC constexpr U64 secret1 = 0xe7037ed1a0b428dbull;
	NH_HEADER_STATIC constexpr U64 secret2 = 0x8ebc6af09c88c6e3ull;
	NH_HEADER_STATIC constexpr U64 secret3 = 0x589965cc75374cc3ull;

	static inline void Multiply(U64& a, U64& b)
	{
#if defined __SIZEOF_INT128__
		__uint128_t r = a;
		r *= b;
		a = (U64)r;
		b = (U64)(r >> 64);
#elif defined _MSC_VER
		a = _umul128(a, b, &b);
#else
		U64 ha = a >> 32, hb = b >> 32, la = (U32)A, lb = (U32)B, hi, lo;
		U64 rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
		lo = t + (rm1 << 32);
		c += lo < t;
		hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
		a = lo;
		b = hi;
#endif
	}

	static inline U64 Mix(U64 A, U64 B) { Multiply(A, B); return A ^ B; }

#if defined NH_LITTLE_ENDIAN
	static inline U64 Read8(const U8* p) { U64 v; memcpy(&v, p, 8); return v; }
	static inline U64 Read4(const U8* p) { U32 v; memcpy(&v, p, 4); return v; }
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
	static inline U64 Read8(const U8* p) { U64 v; memcpy(&v, p, 8); return __builtin_bswap64(v); }
	static inline U64 Read4(const U8* p) { U32 v; memcpy(&v, p, 4); return __builtin_bswap32(v); }
#elif defined _MSC_VER
	static inline U64 Read8(const U8* p) { U64 v; memcpy(&v, p, 8); return _byteswap_uint64(v); }
	static inline U64 Read4(const U8* p) { U32 v; memcpy(&v, p, 4); return _byteswap_ulong(v); }
#else
	static inline U64 Read8(const U8* p)
	{
		U64 v;
		memcpy(&v, p, 8);
		return (((v >> 56) & 0xff) | ((v >> 40) & 0xff00) | ((v >> 24) & 0xff0000) | ((v >> 8) & 0xff000000) | ((v << 8) & 0xff00000000) | ((v << 24) & 0xff0000000000) | ((v << 40) & 0xff000000000000) | ((v << 56) & 0xff00000000000000));
	}
	static inline U64 Read4(const U8* p)
	{
		U32 v;
		memcpy(&v, p, 4);
		return (((v >> 24) & 0xff) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000));
	}
#endif

	static inline U64 Read3(const U8* p, U64 k) { return (((U64)p[0]) << 16) | (((U64)p[k >> 1]) << 8) | p[k - 1]; }
}

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
/// <param name="seed:"></param>
/// <returns></returns>
template<class Type>
static inline U64 Hash(const Type& value, U64 seed = 0)
{
	constexpr U64 length = sizeof(Type);

	const U8* p = (const U8*)&value;
	seed ^= detail::Mix(seed ^ detail::secret0, detail::secret1);

	U64	a, b;
	if constexpr (length <= 16)
	{
		if constexpr (length >= 4)
		{
			a = (detail::Read4(p) << 32) | detail::Read4(p + ((length >> 3) << 2));
			b = (detail::Read4(p + length - 4) << 32) | detail::Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if constexpr (length > 0) { a = detail::Read3(p, length); b = 0; }
		else { a = b = 0; }
	}
	else
	{
		U64 i = length;
		if constexpr (length > 48)
		{
			U64 seed1 = seed, seed2 = seed;
			do
			{
				seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
				seed1 = detail::Mix(detail::Read8(p + 16) ^ detail::secret2, detail::Read8(p + 24) ^ seed1);
				seed2 = detail::Mix(detail::Read8(p + 32) ^ detail::secret3, detail::Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = detail::Read8(p + i - 16);
		b = detail::Read8(p + i - 8);
	}

	a ^= detail::secret1;
	b ^= seed;
	detail::Multiply(a, b);
	return detail::Mix(a ^ detail::secret0 ^ length, b ^ detail::secret1);
}

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
/// <param name="seed:"></param>
/// <returns></returns>
template <U64 length>
static inline U64 Hash(const char(&value)[length], U64 seed = 0)
{
	const U8* p = (const U8*)&value;
	seed ^= detail::Mix(seed ^ detail::secret0, detail::secret1);

	U64	a, b;
	if constexpr (length <= 16)
	{
		if constexpr (length >= 4)
		{
			a = (detail::Read4(p) << 32) | detail::Read4(p + ((length >> 3) << 2));
			b = (detail::Read4(p + length - 4) << 32) | detail::Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if constexpr (length > 0) { a = detail::Read3(p, length); b = 0; }
		else { a = b = 0; }
	}
	else
	{
		U64 i = length;
		if constexpr (length > 48)
		{
			U64 seed1 = seed, seed2 = seed;
			do
			{
				seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
				seed1 = detail::Mix(detail::Read8(p + 16) ^ detail::secret2, detail::Read8(p + 24) ^ seed1);
				seed2 = detail::Mix(detail::Read8(p + 32) ^ detail::secret3, detail::Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = detail::Read8(p + i - 16);
		b = detail::Read8(p + i - 8);
	}

	a ^= detail::secret1;
	b ^= seed;
	detail::Multiply(a, b);
	return detail::Mix(a ^ detail::secret0 ^ length, b ^ detail::secret1);
}

/// <summary>
/// 
/// </summary>
/// <param name="value:"></param>
/// <param name="length:"></param>
/// <param name="seed:"></param>
/// <returns></returns>
static inline U64 Hash(const void* value, U64 length, U64 seed = 0)
{
	const U8* p = (const U8*)&value;
	seed ^= detail::Mix(seed ^ detail::secret0, detail::secret1);

	U64	a, b;
	if (length <= 16)
	{
		if (length >= 4)
		{
			a = (detail::Read4(p) << 32) | detail::Read4(p + ((length >> 3) << 2));
			b = (detail::Read4(p + length - 4) << 32) | detail::Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if (length > 0) { a = detail::Read3(p, length); b = 0; }
		else { a = b = 0; }
	}
	else
	{
		U64 i = length;
		if (length > 48)
		{
			U64 seed1 = seed, seed2 = seed;
			do
			{
				seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
				seed1 = detail::Mix(detail::Read8(p + 16) ^ detail::secret2, detail::Read8(p + 24) ^ seed1);
				seed2 = detail::Mix(detail::Read8(p + 32) ^ detail::secret3, detail::Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = detail::Mix(detail::Read8(p) ^ detail::secret1, detail::Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = detail::Read8(p + i - 16);
		b = detail::Read8(p + i - 8);
	}

	a ^= detail::secret1;
	b ^= seed;
	detail::Multiply(a, b);
	return detail::Mix(a ^ detail::secret0 ^ length, b ^ detail::secret1);
}