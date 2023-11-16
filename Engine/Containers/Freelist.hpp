#pragma once

#include "ContainerDefines.hpp"

//TODO: Resizing
struct NH_API Freelist
{
public:
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
	U32 capacity;
	bool outsideAllocated;

	U32 freeCount;
	U32* freeIndices;
	U32 lastFree;
};