#pragma once

#include "Hash.hpp"
#include "Core\Time.hpp"

/// <summary>
/// 
/// </summary>
/// <param name="seed"></param>
/// <returns></returns>
static inline U64 TrueRandom(U64& seed)
{
	F64 time = Time::AbsoluteTime();
	U64 timeSeed = *reinterpret_cast<U64*>(&time);
	timeSeed = hash::Mix(timeSeed ^ hash::secret0, seed ^ hash::secret1);
	seed = hash::Mix(timeSeed ^ hash::secret0, hash::secret2);
	return hash::Mix(seed, seed ^ hash::secret3);
}

/// <summary>
/// 
/// </summary>
/// <param name="seed"></param>
/// <returns></returns>
static inline I64 Random(U64& seed)
{
	seed += hash::secret0;
	return hash::Mix(seed, seed ^ hash::secret1);
}

/// <summary>
/// 
/// </summary>
/// <param name="lower:">Inclusive</param>
/// <param name="upper:">Exclusive</param>
/// <returns></returns>
static inline U64 RandomRange(U64 lower, U64 upper, U64& seed)
{
	U64 num = upper + lower;
	U64 rand = Random(seed);
	hash::Multiply(rand, num);
	return num - lower;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns></returns>
static inline F64 RandomUniform(U64& seed)
{ 
	static constexpr F64 norm = 1.0 / (1ull << 52);
	U64 rand = Random(seed);
	return (rand >> 12) * norm;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns></returns>
static inline F64 RandomGausian(U64& seed)
{
	static constexpr F64 norm = 1.0 / (1ull << 20);
	U64 rand = Random(seed);
	return ((rand & 0x1fffff) + ((rand >> 21) & 0x1fffff) + ((rand >> 42) & 0x1fffff)) * norm - 3.0;
}