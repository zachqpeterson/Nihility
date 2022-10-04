#include "World.hpp"

#include "GridBroadphase.hpp"
#include "Tile.h"
#include "Chunk.hpp"

#include <Physics/Physics.hpp>
#include <Memory/Memory.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/Camera.hpp>

#define VIEW_DISTANCE_X 3
#define VIEW_DISTANCE_Y 2

World::World(I64 seed, WorldSize size) : SEED{ seed }, TILES_X{ (U16)size }, TILES_Y{ (U16)(TILES_X / 3.5f) }, CHUNKS_X{ (U16)(TILES_X / CHUNK_SIZE) }, CHUNKS_Y{ (U16)(TILES_Y / CHUNK_SIZE) }
{
	Math::SeedRandom((U32)seed);

	tiles = (Tile**)Memory::LinearAllocate(sizeof(Tile*) * TILES_X);

	for (U16 i = 0; i < TILES_X; ++i)
	{
		tiles[i] = (Tile*)Memory::LinearAllocate(sizeof(Tile) * TILES_Y);
	}

	GenerateWorld();

	chunks = (Chunk**)Memory::LinearAllocate(sizeof(Chunk*) * CHUNKS_X);

	for (U16 x = 0; x < CHUNKS_X; ++x)
	{
		chunks[x] = (Chunk*)Memory::LinearAllocate(sizeof(Chunk) * CHUNKS_Y);

		U16 tileX = x * CHUNK_SIZE;

		for (U16 y = 0; y < CHUNKS_Y; ++y)
		{
			U16 tileY = y * CHUNK_SIZE;

			Tile** chunkTiles = tiles + tileX;

			for (U16 i = 0; i < CHUNK_SIZE; ++i)
			{
				chunkTiles[i] = tiles[tileX] + tileY;
			}

			chunks[x][y].SetTiles(chunkTiles);
		}
	}

	U16 xPos = CHUNKS_X / 2 - 2;
	U16 yPos = 2;

	RendererFrontend::CurrentScene()->GetCamera()->SetPosition(Vector3{TILES_X / 2.0f, 16.0f, 10.0f}); //TODO: set player position and set player as camera target

	chunks[xPos][yPos].Load(Vector2{ (F32)xPos, (F32)yPos });
	chunks[xPos + 1][yPos].Load(Vector2{ (F32)xPos + 0.88888888888f, (F32)yPos });

	//for (U16 x = xPos - VIEW_DISTANCE_X; x <= xPos + VIEW_DISTANCE_X; ++x)
	//{
	//	for (U16 y = yPos - VIEW_DISTANCE_Y; y <= yPos + VIEW_DISTANCE_Y; ++y)
	//	{
	//		chunks[x][y].Load(Vector2{ (F32)x, (F32)y });
	//	}
	//}

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
			(Math::Simplex1(x * 0.05 + SEED) * 5.0) + (TILES_Y / 2.0));

		for (U16 y = 0; y < TILES_Y; ++y)
		{
			tiles[x][y].blockID = 1;
		}
	}
}
