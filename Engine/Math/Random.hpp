#pragma once

#include "Defines.hpp"

#include "Core/Time.hpp"

//Based on wyrand - https://github.com/wangyi-fudan/wyhash
class NH_API Random
{
	static constexpr inline U64 secret0 = 0xa0761d6478bd642full;
	static constexpr inline U64 secret1 = 0xe7037ed1a0b428dbull;
	static constexpr inline U64 secret2 = 0x8ebc6af09c88c6e3ull;
	static constexpr inline U64 secret3 = 0x589965cc75374cc3ull;

public:
	/// <summary>
	/// Creates a completely unpredictable random integer
	/// </summary>
	/// <returns>The random number</returns>
	static U64 TrueRandomInt()
	{
		I64 time = Time::CoreCounter();
		time = Mix(time ^ secret0, seed ^ secret1);
		seed = Mix(time ^ secret0, secret2);
		return Mix(seed, seed ^ secret3);
	}


	/// <summary>
	/// Creates a deterministically random integer based on the current seed
	/// </summary>
	/// <returns>The random number</returns>
	static U64 RandomInt()
	{
		seed += secret0;
		return Mix(seed, seed ^ secret1);
	}

	/// <summary>
	/// Creates a deterministically random integer within the range based on the current seed
	/// </summary>
	/// <param name="lower:">Lower bound, inclusive</param>
	/// <param name="upper:">Upper bound, exclusive</param>
	/// <returns>The random number</returns>
	static U64 RandomRange(U64 lower, U64 upper)
	{
		U64 num = upper + lower;
		U64 rand = RandomInt();
		Multiply(rand, num);
		return num - lower;
	}

	/// <summary>
	/// Creates a deterministically random integer with a uniform distribution based on the current seed
	/// </summary>
	/// <returns>The random number</returns>
	static F64 RandomUniform()
	{
		constexpr F64 norm = 1.0 / (1ull << 52);
		U64 rand = RandomInt();
		return (rand >> 12) * norm;
	}

	/// <summary>
	/// Sets the seed used to generate random numbers, seed get automatically updated whenever any of these functions are called
	/// </summary>
	/// <param name="newSeed:">The new seed value</param>
	static void SeedRandom(U64 newSeed) { seed = newSeed; }

private:
	static inline U64 seed = 0;

	static void Multiply(U64& a, U64& b)
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

	static U64 Mix(U64 a, U64 b) { Multiply(a, b); return a ^ b; }
	
	STATIC_CLASS(Random);
};