#include "Hash.hpp"

#include "Memory\Memory.hpp"

void Hash::Multiply(U64& a, U64& b)
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

U64 Hash::Mix(U64 A, U64 B) { Multiply(A, B); return A ^ B; }

U64 Hash::Read8(const U8* p)
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

U64 Hash::Read4(const U8* p)
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

U64 Hash::Read3(const U8* p, U64 k) { return (((U64)p[0]) << 16) | (((U64)p[k >> 1]) << 8) | p[k - 1]; }