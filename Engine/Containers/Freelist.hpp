#pragma once

#include "ContainerDefines.hpp"

//TODO: Resizing
struct Freelist
{
public:
	Freelist();
	Freelist(U32 count);

	void Destroy();
	~Freelist();

	Freelist& operator()(U32 count);
	Freelist& operator()(U32* memory, U32 count);

	U32 GetFree();
	void Release(U32 index);

	bool Full() const;

private:
	U32 size;
	U32 capacity;
	bool outsideAllocated;

	U32 freeCount;
	U32* freeIndices;
	U32 lastFree;
};