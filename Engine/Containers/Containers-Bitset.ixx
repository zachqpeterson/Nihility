module;

#include "Defines.hpp"

export module Containers:Bitset;

import Memory;

export struct Bitset
{
public:
	void Create(U32 bitCapacity);
	void Destroy();

	void SetBitCountAndClear(U32 bitCount);
	void GrowBitSet(U32 blockCount);
	void InPlaceUnion(const Bitset& other);
	void SetBit(U32 bitIndex);
	void SetBitGrow(U32 bitIndex);
	void ClearBit(U32 bitIndex);
	bool GetBit(U32 bitIndex) const;
	I32 GetBitSetBytes();

public:
	U64* bits = nullptr;
	U32 blockCapacity = 0;
	U32 blockCount = 0;
};

void Bitset::Create(U32 bitCapacity)
{
	blockCapacity = (bitCapacity + sizeof(U64) * 8 - 1) / (sizeof(U64) * 8);
	blockCount = 0;
	Memory::AllocateArray(&bits, blockCapacity);
}

void Bitset::Destroy()
{
	Memory::Free(&bits);
	blockCapacity = 0;
	blockCount = 0;
}

void Bitset::SetBitCountAndClear(U32 bitCount)
{
	U32 blockCount = (bitCount + sizeof(U64) * 8 - 1) / (sizeof(U64) * 8);
	if (blockCapacity < blockCount)
	{
		Memory::Free(&bits);
		U32 newBitCapacity = bitCount + (bitCount >> 1);
		Memory::AllocateArray(&bits, newBitCapacity);
	}

	blockCount = blockCount;
	Zero(bits, blockCount * sizeof(U64));
}

void Bitset::GrowBitSet(U32 blockCount)
{
	if (blockCount > blockCapacity)
	{
		U32 oldCapacity = blockCapacity;
		blockCapacity = blockCount + blockCount / 2;
		Memory::Reallocate(&bits, blockCapacity);
	}

	this->blockCount = blockCount;
}

void Bitset::InPlaceUnion(const Bitset& other)
{
	for (U32 i = 0; i < blockCount; ++i)
	{
		bits[i] |= other.bits[i];
	}
}

void Bitset::SetBit(U32 bitIndex)
{
	U32 blockIndex = bitIndex / 64;
	bits[blockIndex] |= ((U64)1 << bitIndex % 64);
}

void Bitset::SetBitGrow(U32 bitIndex)
{
	U32 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { GrowBitSet(blockIndex + 1); }
	bits[blockIndex] |= ((U64)1 << bitIndex % 64);
}

void Bitset::ClearBit(U32 bitIndex)
{
	U32 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return; }
	bits[blockIndex] &= ~((U64)1 << bitIndex % 64);
}

bool Bitset::GetBit(U32 bitIndex) const
{
	U32 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return false; }
	return (bits[blockIndex] & ((U64)1 << bitIndex % 64)) != 0;
}

I32 Bitset::GetBitSetBytes()
{
	return blockCapacity * sizeof(U64);
}
