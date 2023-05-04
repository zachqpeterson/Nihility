#pragma once

#include "Defines.hpp"

#include <stdint.h>
#include <string.h>
#ifdef _MSC_VER
#pragma intrinsic(_umul128)
#endif

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

static inline U64 wyhash(const void* key, U64 len, U64 seed)
{
	const U8* p = (const U8*)key;
	seed ^= Mix(seed ^ secret0, secret1);
	U64	a, b;
	if (len <= 16)
	{
		if (len >= 4)
		{
			a = (Read4(p) << 32) | Read4(p + ((len >> 3) << 2));
			b = (Read4(p + len - 4) << 32) | Read4(p + len - 4 - ((len >> 3) << 2));
		}
		else if (len > 0) { a = Read3(p, len); b = 0; }
		else { a = b = 0; }
	}
	else
	{
		U64 i = len;
		if (i > 48)
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

		while (i > 16) { seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed); i -= 16; p += 16; }

		a = Read8(p + i - 16);
		b = Read8(p + i - 8);
	}

	a ^= secret1;
	b ^= seed;
	Multiply(a, b);
	return Mix(a ^ secret0 ^ len, b ^ secret1);
}

//a useful 64bit-64bit mix function to produce deterministic pseudo random numbers that can pass BigCrush and PractRand
static inline U64 wyhash64(U64 A, U64 B) { A ^= secret0; B ^= secret1; Multiply(A, B); return Mix(A ^ secret0, B ^ secret1); }

//The wyrand PRNG that pass BigCrush and PractRand
static inline U64 wyrand(U64* seed) { *seed += secret0; return Mix(*seed, *seed ^ secret1); }

//convert any 64 bit pseudo random numbers to uniform distribution [0,1). It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2u01(U64 r) { const double _wynorm = 1.0 / (1ull << 52); return (r >> 12) * _wynorm; }

//convert any 64 bit pseudo random numbers to APPROXIMATE Gaussian distribution. It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2gau(U64 r) { const double _wynorm = 1.0 / (1ull << 20); return ((r & 0x1fffff) + ((r >> 21) & 0x1fffff) + ((r >> 42) & 0x1fffff)) * _wynorm - 3.0; }

#ifdef	WYTRNG
#include <sys/time.h>
//The wytrand true random number generator, passed BigCrush.
static inline U64 wytrand(U64* seed)
{
	struct	timeval	t;	gettimeofday(&t, 0);
	U64	teed = (((U64)t.tv_sec) << 32) | t.tv_usec;
	teed = _wymix(teed ^ _wyp[0], *seed ^ _wyp[1]);
	*seed = _wymix(teed ^ _wyp[0], _wyp[2]);
	return _wymix(*seed, *seed ^ _wyp[3]);
}
#endif

//fast range integer random number generation on [0,k) credit to Daniel Lemire. May not work when WYHASH_32BIT_MUM=1. It can be combined with wyrand, wyhash64 or wyhash.
static inline U64 wy2u0k(U64 r, U64 k) { Multiply(r, k); return k; }

template<class Type>
static inline U64 Hash(const Type& value, U64 seed = 0)
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