#pragma once

#include "ContainerDefines.hpp"

//TODO: Resizing
struct NH_API Freelist
{
public:
	Freelist(NullPointer);
	Freelist();
	Freelist(U32 count);

	Freelist& operator()(U32 count);
	Freelist& operator()(U32* memory, U32 count);

	void Destroy();
	~Freelist();

	void Reset();

	U32 GetFree();
	void Release(U32 index);

	bool Full() const;

private:
	U32 capacity{ 0 };
	bool outsideAllocated{ false };

	U32 freeCount{ 0 };
	U32* freeIndices{ nullptr };
	U32 lastFree{ 0 };
};