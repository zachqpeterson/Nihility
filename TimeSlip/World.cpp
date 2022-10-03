#include "World.hpp"

#include "GridBroadphase.hpp"
#include "Tile.h"
#include "Chunk.hpp"

#include <Physics/Physics.hpp>
#include <Memory/Memory.hpp>

World::World(I64 seed, WorldSize size) : SEED{ seed }, TILES_X{ (U16)size }, TILES_Y{ (U16)(TILES_X / 3.5f) }, CHUNKS_X{ (U16)(TILES_X / 8) }, CHUNKS_Y{ (U16)(TILES_Y / 8) }
{
	Math::SeedRandom((U32)seed);

	tiles = (Tile**)Memory::LinearAllocate(sizeof(Tile*) * TILES_Y);

	for (U16 i = 0; i < TILES_X; ++i)
	{
		tiles[i] = (Tile*)Memory::LinearAllocate(sizeof(Tile));
	}

	GenerateWorld();

	chunks = (Chunk**)Memory::LinearAllocate(sizeof(Chunk*) * CHUNKS_Y);

	for (U16 i = 0; i < CHUNKS_X; ++i)
	{
		chunks[i] = (Chunk*)Memory::LinearAllocate(sizeof(Chunk));

		chunks[i];
	}

	GridBroadphase* bp = new GridBroadphase(TILES_X, TILES_Y);
	Physics::SetBroadphase(bp);
}

World::~World()
{

}

void World::Destroy()
{

}

void* World::operator new(U64 size) { return Memory::Allocate(sizeof(World), MEMORY_TAG_GAME); }

void World::operator delete(void* ptr) { Memory::Free(ptr, sizeof(World), MEMORY_TAG_GAME); }

void World::GenerateWorld()
{
	for (U16 x = 0; x < TILES_X; ++x)
	{
		U16 height = (U16)((Math::Simplex1(x * 0.005 + SEED) * 25.0) +
			(Math::Simplex1(x * 0.05 + SEED) * 5.0) + (TILES_Y / 2.0f));

		for (U16 y = 0; y < TILES_Y; ++y)
		{
			tiles[x][y].blockID = 1;
		}
	}
}
