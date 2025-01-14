#include "Bitset.hpp"

#include "Memory/Memory.hpp"

void Bitset::Create(U64 bitCapacity)
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

void Bitset::SetBitCountAndClear(U64 bitCount)
{
	U64 blockCount = (bitCount + sizeof(U64) * 8 - 1) / (sizeof(U64) * 8);
	if (blockCapacity < blockCount)
	{
		Memory::Free(&bits);
		U64 newBitCapacity = bitCount + (bitCount >> 1);
		Memory::AllocateArray(&bits, newBitCapacity);
	}

	blockCount = blockCount;
	Zero(bits, blockCount * sizeof(U64));
}

void Bitset::GrowBitSet(U64 blockCount)
{
	if (blockCount > blockCapacity)
	{
		U64 oldCapacity = blockCapacity;
		blockCapacity = blockCount + blockCount / 2;
		Memory::Reallocate(&bits, blockCapacity);
	}

	this->blockCount = blockCount;
}

void Bitset::InPlaceUnion(const Bitset& other)
{
	for (U64 i = 0; i < blockCount; ++i)
	{
		bits[i] |= other.bits[i];
	}
}

void Bitset::SetBit(U64 bitIndex)
{
	U64 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return; }
	bits[blockIndex] |= ((U64)1 << bitIndex % 64);
}

void Bitset::SetBitGrow(U64 bitIndex)
{
	U64 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { GrowBitSet(blockIndex + 1); }
	bits[blockIndex] |= ((U64)1 << bitIndex % 64);
}

void Bitset::ClearBit(U64 bitIndex)
{
	U64 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return; }
	bits[blockIndex] &= ~((U64)1 << bitIndex % 64);
}

bool Bitset::GetBit(U64 bitIndex) const
{
	U64 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return false; }
	return (bits[blockIndex] & ((U64)1 << bitIndex % 64)) != 0;
}

bool Bitset::operator[](U64 bitIndex) const
{
	U64 blockIndex = bitIndex / 64;
	if (blockIndex >= blockCount) { return false; }
	return (bits[blockIndex] & ((U64)1 << bitIndex % 64)) != 0;
}

U64 Bitset::GetBitSetBytes()
{
	return blockCapacity * sizeof(U64);
}