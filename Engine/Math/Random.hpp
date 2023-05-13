// Based on wyrand - https://github.com/wangyi-fudan/wyhash

#pragma once

#include "Hash.hpp"
#include "Core\Time.hpp"

class Random
{
	static U64 TrueRandomInt(U64& seed);
	static U64 RandomInt(U64& seed);
	static U64 RandomRange(U64 lower, U64 upper, U64& seed);
	static F64 RandomUniform(U64& seed);
	static F64 RandomGausian(U64& seed);

private:
	STATIC_CLASS(Random);
};

/// <summary>
/// 
/// </summary>
/// <param name="seed"></param>
/// <returns></returns>
inline U64 Random::TrueRandomInt(U64& seed)
{
	F64 time = Time::AbsoluteTime();
	U64 timeSeed = *reinterpret_cast<U64*>(&time);
	timeSeed = Hash::Mix(timeSeed ^ Hash::secret0, seed ^ Hash::secret1);
	seed = Hash::Mix(timeSeed ^ Hash::secret0, Hash::secret2);
	return Hash::Mix(seed, seed ^ Hash::secret3);
}

/// <summary>
/// 
/// </summary>
/// <param name="seed"></param>
/// <returns></returns>
inline U64 Random::RandomInt(U64& seed)
{
	seed += Hash::secret0;
	return Hash::Mix(seed, seed ^ Hash::secret1);
}

/// <summary>
/// 
/// </summary>
/// <param name="lower:">Inclusive</param>
/// <param name="upper:">Exclusive</param>
/// <returns></returns>
inline U64 Random::RandomRange(U64 lower, U64 upper, U64& seed)
{
	U64 num = upper + lower;
	U64 rand = RandomInt(seed);
	Hash::Multiply(rand, num);
	return num - lower;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns></returns>
inline F64 Random::RandomUniform(U64& seed)
{ 
	static constexpr F64 norm = 1.0 / (1ull << 52);
	U64 rand = RandomInt(seed);
	return (rand >> 12) * norm;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns></returns>
inline F64 Random::RandomGausian(U64& seed)
{
	static constexpr F64 norm = 1.0 / (1ull << 20);
	U64 rand = RandomInt(seed);
	return ((rand & 0x1fffff) + ((rand >> 21) & 0x1fffff) + ((rand >> 42) & 0x1fffff)) * norm - 3.0;
}