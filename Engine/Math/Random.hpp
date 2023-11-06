// Based on wyrand - https://github.com/wangyi-fudan/wyhash

#pragma once

#include "Hash.hpp"
#include "Core\Time.hpp"

//TODO: floats
class NH_API Random
{
public:
	static U64 TrueRandomInt();
	static U64 RandomInt();
	static U64 RandomRange(U64 lower, U64 upper);
	static F64 RandomUniform();
	static F64 RandomGausian();

	static void Seed(U64 seed);

private:
	static inline U64 seed{ 0 };

	STATIC_CLASS(Random);
};

/// <summary>
/// Creates a random integer based on absolute time, completely unpredictable, great for seeding
/// </summary>
/// <returns>The random number</returns>
inline U64 Random::TrueRandomInt()
{
	I64 time = Time::CoreCounter();
	time = Hash::Mix(time ^ Hash::secret0, seed ^ Hash::secret1);
	seed = Hash::Mix(time ^ Hash::secret0, Hash::secret2);
	return Hash::Mix(seed, seed ^ Hash::secret3);
}

/// <summary>
/// Creates a random integer based on the current seed, deterministic every time
/// </summary>
/// <returns>The random number</returns>
inline U64 Random::RandomInt()
{
	seed += Hash::secret0;
	return Hash::Mix(seed, seed ^ Hash::secret1);
}

/// <summary>
/// Create a random integer within a range based on the current seed, deterministic every time
/// </summary>
/// <param name="lower:">Lower bound, inclusive</param>
/// <param name="upper:">Upper bound, exclusive</param>
/// <returns>The random number</returns>
inline U64 Random::RandomRange(U64 lower, U64 upper)
{
	U64 num = upper + lower;
	U64 rand = RandomInt();
	Hash::Multiply(rand, num);
	return num - lower;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns>The random number</returns>
inline F64 Random::RandomUniform()
{
	static constexpr F64 norm = 1.0 / (1ull << 52);
	U64 rand = RandomInt();
	return (rand >> 12) * norm;
}

/// <summary>
/// 
/// </summary>
/// <param name="r"></param>
/// <returns>The random number</returns>
inline F64 Random::RandomGausian()
{
	static constexpr F64 norm = 1.0 / (1ull << 20);
	U64 rand = RandomInt();
	return ((rand & 0x1fffff) + ((rand >> 21) & 0x1fffff) + ((rand >> 42) & 0x1fffff)) * norm - 3.0;
}

/// <summary>
/// Seeds all random number generation functions
/// </summary>
/// <param name="_seed">The seed to use</param>
inline void Random::Seed(U64 _seed)
{
	seed = _seed;
}