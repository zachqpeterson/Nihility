// Based on wyhash - https://github.com/wangyi-fudan/wyhash

#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"

#ifdef _MSC_VER
#pragma intrinsic(_umul128)
#endif

class Hash
{
public:
	template<class Type> requires(!IsPointer<Type>) static inline U64 Calculate(const Type& value, U64 seed = 0);
	template <class Type, U64 length> static inline U64 Calculate(const Type(&value)[length], U64 seed = 0);
	template<class Type> static inline U64 Calculate(const Type* value, U64 length, U64 seed = 0);

private:
	static void Multiply(U64& a, U64& b);
	static U64 Mix(U64 A, U64 B);

	static inline U64 Read8(const U8* p);
	static inline U64 Read4(const U8* p);
	static inline U64 Read3(const U8* p, U64 k);

	NH_HEADER_STATIC constexpr U64 secret0 = 0xa0761d6478bd642full;
	NH_HEADER_STATIC constexpr U64 secret1 = 0xe7037ed1a0b428dbull;
	NH_HEADER_STATIC constexpr U64 secret2 = 0x8ebc6af09c88c6e3ull;
	NH_HEADER_STATIC constexpr U64 secret3 = 0x589965cc75374cc3ull;

	STATIC_CLASS(Hash);
	friend class Random;
};

inline void Hash::Multiply(U64& a, U64& b)
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

inline U64 Hash::Mix(U64 A, U64 B) { Multiply(A, B); return A ^ B; }

inline U64 Hash::Read8(const U8* p)
{
#if defined NH_LITTLE_ENDIAN
	U64 v;
	Memory::Copy(&v, p, 8);
	return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
	U64 v;
	Memory::Copy(&v, p, 8);
	return __builtin_bswap64(v);
#elif defined _MSC_VER
	U64 v;
	Memory::Copy(&v, p, 8);
	return _byteswap_uint64(v);
#else
	U64 v;
	Memory::Copy(&v, p, 8);
	return (((v >> 56) & 0xff) | ((v >> 40) & 0xff00) | ((v >> 24) & 0xff0000) | ((v >> 8) & 0xff000000) | 
		((v << 8) & 0xff00000000) | ((v << 24) & 0xff0000000000) | ((v << 40) & 0xff000000000000) | ((v << 56) & 0xff00000000000000));
#endif
}

inline U64 Hash::Read4(const U8* p)
{
#if defined NH_LITTLE_ENDIAN
	U32 v;
	Memory::Copy(&v, p, 4);
	return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
	U32 v;
	Memory::Copy(&v, p, 4);
	return __builtin_bswap32(v);
#elif defined _MSC_VER
	U32 v;
	Memory::Copy(&v, p, 4);
	return _byteswap_ulong(v);
#else
	U32 v;
	Memory::Copy(&v, p, 4);
	return (((v >> 24) & 0xff) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000));
#endif
}

inline U64 Hash::Read3(const U8* p, U64 k) { return (((U64)p[0]) << 16) | (((U64)p[k >> 1]) << 8) | p[k - 1]; }

template<class Type> requires(!IsPointer<Type>)
inline U64 Hash::Calculate(const Type& value, U64 seed)
{
	constexpr U64 length = sizeof(Type);

	const U8* p = (const U8*)&value;
	seed ^= Mix(seed ^ secret0, secret1);

	U64	a, b;
	if constexpr (length <= 16)
	{
		if constexpr (length >= 4)
		{
			a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
			b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if constexpr (length > 0) { a = Read3(p, length); b = 0; }
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
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
				seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = Read8(p + i - 16);
		b = Read8(p + i - 8);
	}

	a ^= secret1;
	b ^= seed;
	Multiply(a, b);
	return Mix(a ^ secret0 ^ length, b ^ secret1);
}

template <class Type, U64 len>
inline U64 Hash::Calculate(const Type(&value)[len], U64 seed)
{
	constexpr U64 length = len * sizeof(Type);
	const U8* p = (const U8*)value;
	seed ^= Mix(seed ^ secret0, secret1);

	U64	a, b;
	if constexpr (length <= 16)
	{
		if constexpr (length >= 4)
		{
			a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
			b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if constexpr (length > 0) { a = Read3(p, length); b = 0; }
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
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
				seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = Read8(p + i - 16);
		b = Read8(p + i - 8);
	}

	a ^= secret1;
	b ^= seed;
	Multiply(a, b);
	return Mix(a ^ secret0 ^ length, b ^ secret1);
}

template<class Type>
inline U64 Hash::Calculate(const Type* value, U64 len, U64 seed)
{
	const U64 length = len * sizeof(Type);
	const U8* p = (const U8*)value;
	seed ^= Mix(seed ^ secret0, secret1);

	U64	a, b;
	if (length <= 16)
	{
		if (length >= 4)
		{
			a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
			b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
		}
		else if (length > 0) { a = Read3(p, length); b = 0; }
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
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
				seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
				p += 48;
				i -= 48;
			} while (i > 48);

			seed ^= seed1 ^ seed2;
		}

		while (i > 16)
		{
			seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}

		a = Read8(p + i - 16);
		b = Read8(p + i - 8);
	}

	a ^= secret1;
	b ^= seed;
	Multiply(a, b);
	return Mix(a ^ secret0 ^ length, b ^ secret1);
}