#pragma once

#include "Defines.hpp"

//TODO: Resizing
struct NH_API Freelist
{
public:
	Freelist(NullPointer);
	Freelist();
	Freelist(U32 count);

	Freelist& operator()(U32 count);
	Freelist& operator()(U32* memory, U32 count);

	~Freelist();
	void Destroy();

	void Reset();

	U32 GetFree();
	void Release(U32 index);

	bool Full() const;
	U32 Size() const;
	U32 Capacity() const;
	U32 Last() const;

	void Resize(U32 count);

private:
	U32 capacity{ 0 };
	U32 used{ 0 };
	bool outsideAllocated{ false };

	U32 freeCount{ 0 };
	U32* freeIndices{ nullptr };
	U32 lastFree{ 0 };
};