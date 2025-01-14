#pragma once

#include "Defines.hpp"

struct NH_API Bitset
{
public:
	void Create(U64 bitCapacity);
	void Destroy();

	void SetBitCountAndClear(U64 bitCount);
	void GrowBitSet(U64 blockCount);
	void InPlaceUnion(const Bitset& other);
	void SetBit(U64 bitIndex);
	void SetBitGrow(U64 bitIndex);
	void ClearBit(U64 bitIndex);
	bool GetBit(U64 bitIndex) const;
	U64 GetBitSetBytes();

	bool operator[](U64 bitIndex) const;

public:
	U64* bits = nullptr;
	U64 blockCapacity = 0;
	U64 blockCount = 0;
};