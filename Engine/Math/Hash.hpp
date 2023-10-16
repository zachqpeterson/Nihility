// Based on wyhash - https://github.com/wangyi-fudan/wyhash

#pragma once

#include "Defines.hpp"

class Hash
{
public:
	template<class Type> requires(!IsPointer<Type>) static U64 Calculate(const Type& value, U64 seed = 0);
	template<class Type, U64 length> static U64 Calculate(const Type(&value)[length], U64 seed = 0);
	template<class Type> static U64 Calculate(const Type* value, U64 length, U64 seed = 0);

private:
	static void Multiply(U64& a, U64& b);
	static U64 Mix(U64 A, U64 B);

	static U64 Read8(const U8* p);
	static U64 Read4(const U8* p);
	static U64 Read3(const U8* p, U64 k);

	static constexpr U64 secret0 = 0xa0761d6478bd642full;
	static constexpr U64 secret1 = 0xe7037ed1a0b428dbull;
	static constexpr U64 secret2 = 0x8ebc6af09c88c6e3ull;
	static constexpr U64 secret3 = 0x589965cc75374cc3ull;

	STATIC_CLASS(Hash);
	friend class Random;
};

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