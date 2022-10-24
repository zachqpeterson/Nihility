#include "World.hpp"

#include "GridBroadphase.hpp"
#include "Tile.hpp"
#include "Chunk.hpp"
#include "TimeSlip.hpp"

#include <Physics/Physics.hpp>
#include <Memory/Memory.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/Camera.hpp>
#include <Containers/Array.hpp>
#include <Core/Time.hpp>
#include <Core/Input.hpp>
#include <Platform/Platform.hpp>

#define VIEW_DISTANCE_X 6
#define VIEW_DISTANCE_Y 3

World::World(I64 seed, WorldSize size, Vector2& spawnPoint) : SEED{ seed }, TILES_X{ (U16)size }, TILES_Y{ (U16)(TILES_X / 3.5f) }, CHUNKS_X{ (U16)(TILES_X / CHUNK_SIZE) }, CHUNKS_Y{ (U16)(TILES_Y / CHUNK_SIZE) }
{
	Math::SeedRandom((U32)seed);
	Chunk::world = this;

	tiles = (Tile**)Memory::LinearAllocate(sizeof(Tile*) * TILES_X);

	for (U16 i = 0; i < TILES_X; ++i)
	{
		tiles[i] = (Tile*)Memory::LinearAllocate(sizeof(Tile) * TILES_Y);
	}

	F32 spawnHeight = GenerateWorld();

	chunks = (Chunk**)Memory::LinearAllocate(sizeof(Chunk*) * CHUNKS_X);

	for (U16 x = 0; x < CHUNKS_X; ++x)
	{
		chunks[x] = (Chunk*)Memory::LinearAllocate(sizeof(Chunk) * CHUNKS_Y);

		U16 tileX = x * CHUNK_SIZE;

		for (U16 y = 0; y < CHUNKS_Y; ++y)
		{
			U16 tileY = y * CHUNK_SIZE;

			Array<Tile*, CHUNK_SIZE>& chunkTiles = chunks[x][y].SetTiles();

			for (U16 i = 0; i < CHUNK_SIZE; ++i)
			{
				chunkTiles[i] = tiles[tileX + i] + tileY;
			}
		}
	}

	spawnPoint = { TILES_X * 0.5f, spawnHeight };

	GridBroadphase* bp = new GridBroadphase(tiles, TILES_X, TILES_Y);
	Physics::SetBroadphase(bp);
}

World::~World()
{
	Destroy();
}

void World::Destroy()
{
	for (U16 x = 0; x < CHUNKS_X; ++x)
	{
		for (U16 y = 0; y < CHUNKS_Y; ++y)
		{
			chunks[x][y].Destroy();
		}
	}
}

void* World::operator new(U64 size) { return Memory::Allocate(sizeof(World), MEMORY_TAG_GAME); }

void World::operator delete(void* ptr) { Memory::Free(ptr, sizeof(World), MEMORY_TAG_GAME); }

void World::Update()
{
	static Vector3Int lastPos{ 10, 10, 0 };
	Vector3 pos = (RendererFrontend::CurrentScene()->GetCamera()->Position() / 8.0f);
	Vector3Int posI = (Vector3Int)pos;

	if (posI != lastPos)
	{
		I32 leftMin = Math::Max(lastPos.x - VIEW_DISTANCE_X, 0);
		I32 leftMax = Math::Max(posI.x - VIEW_DISTANCE_X, 0);
		I32 rightMin = Math::Min(posI.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1);
		I32 rightMax = Math::Min(lastPos.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1);
		I32 bottomMin = Math::Max(lastPos.y - VIEW_DISTANCE_Y, 0);
		I32 bottomMax = Math::Max(posI.y - VIEW_DISTANCE_Y, 0);
		I32 topMin = Math::Min(posI.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1);
		I32 topMax = Math::Min(lastPos.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1);

		for (I32 y = bottomMin; y < bottomMax; ++y)
		{
			for (I32 x = leftMin; x <= rightMax; ++x)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 y = topMax; y > topMin; --y)
		{
			for (I32 x = leftMin; x <= rightMax; ++x)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 x = leftMin; x < leftMax; ++x)
		{
			for (I32 y = bottomMin; y <= topMax; ++y)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 x = rightMax; x > rightMin; --x)
		{
			for (I32 y = bottomMin; y <= topMax; ++y)
			{
				chunks[x][y].Unload();
			}
		}

		for (U16 x = leftMax; x <= rightMin; ++x)
		{
			for (U16 y = bottomMax; y <= topMin; ++y)
			{
				chunks[x][y].Load({ x, y });
			}
		}
	}

	//TODO: Block mouse click with the ui
	if (Input::ButtonDown(LEFT_CLICK))
	{
		Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
		Vector2 mousePos = (Vector2)Input::MousePos();
		Vector2 screenSize = (Vector2)Platform::ScreenSize();
		Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
		Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();

		if (Input::ButtonDown(CONTROL))
		{
			BreakWall(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
		else if (Input::ButtonDown(MENU))
		{
			RemoveLight(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
		else
		{
			BreakBlock(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
	}

	if (Input::ButtonDown(RIGHT_CLICK))
	{
		Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
		Vector2 mousePos = (Vector2)Input::MousePos();
		Vector2 screenSize = (Vector2)Platform::ScreenSize();
		Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
		Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();

		if (Input::ButtonDown(CONTROL))
		{
			PlaceWall(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, 1);
		}
		else if (Input::ButtonDown(MENU))
		{
			PlaceLight(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
		else
		{
			PlaceBlock(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, 1);
		}
	}

	lastPos = posI;
}

Vector2 World::BlockUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].blockID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(F32)((pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].blockID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].blockID) << 1) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].blockID) << 2)) };
}

Vector2 World::WallUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].wallID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(F32)((pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].wallID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].wallID) << 1) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].wallID) << 2)) };
}

Vector2 World::DecorationUV(const Vector2Int& pos, U8 id)
{
	if (id == 1)
	{
		return { (F32)(((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3), (F32)tiles[pos.x][pos.y].biome };
	}

	return Vector2((F32)((pos.x > 0 && pos.y > 0 && tiles[pos.x - 1][pos.y - 1].decID == id) +
		((pos.x + 1 < TILES_X && pos.y > 0 && tiles[pos.x + 1][pos.y - 1].decID == id) << 1) +
		((pos.x > 0 && pos.y + 1 < TILES_Y && tiles[pos.x - 1][pos.y + 1].decID == id) << 2) +
		((pos.x + 1 < TILES_X && pos.y + 1 < TILES_Y && tiles[pos.x + 1][pos.y + 1].decID == id) << 3)),
		(F32)((pos.y > 0 && tiles[pos.x][pos.y - 1].decID == id) +
			((pos.x > 0 && tiles[pos.x - 1][pos.y].decID == id) << 1) +
			((pos.x + 1 < TILES_X && tiles[pos.x + 1][pos.y].decID == id) << 2) +
			((pos.y + 1 < TILES_Y && tiles[pos.x][pos.y + 1].decID == id) << 3)));
}

Vector2 World::LiquidUV(const Vector2Int& pos)
{
	return Vector2::ZERO;
}

void World::TileLight(const Vector2Int& pos, Vector3& color, Vector3& globalColor)
{
	color = Vector3::ONE * tiles[pos.x][pos.y].lightSource;
	globalColor = Vector3::ONE * tiles[pos.x][pos.y].globalLightSource;

	U16 maxLightDistance = 8;

	U16 xStart = Math::Max(pos.x - maxLightDistance, 0);
	U16 xEnd = pos.x + maxLightDistance;
	U16 yStart = Math::Max(pos.y - maxLightDistance, 0);
	U16 yEnd = pos.y + maxLightDistance;

	F32 stacking = 1;

	for (U16 x = xStart; x < xEnd && x < TILES_X; ++x)
	{
		for (U16 y = yStart; y < yEnd && y < TILES_Y; ++y)
		{
			if ((x == pos.x && y == pos.y) || (!tiles[x][y].lightSource && !tiles[x][y].globalLightSource) ||
				(pos - Vector2Int{x, y}).SqrMagnitude() > 64) { continue; }

			I16 x1 = pos.x;
			I16 y1 = pos.y;

			I16 dx = x1 - x;
			dx *= Math::Sign(dx);
			I16 dy = y1 - y;
			dy *= Math::Sign(dy);
			I16 x2 = x;
			I16 y2 = y;
			I16 n = 1 + dx + dy;
			I16 x_inc = (x1 > x) ? 1 : -1;
			I16 y_inc = (y1 > y) ? 1 : -1;
			I16 error = dx - dy;
			dx <<= 1;
			dy <<= 1;

			F32 decrDiag = SQRT_TWO_H / 16;
			F32 decrStrait = 1.0f / 16;

			F32 distance = 1.0f;// +(error ? decrStrait : decrDiag);

			bool xGood = (U16)(x2 + x_inc) < TILES_X;
			bool yGood = (U16)(y2 + y_inc) < TILES_Y;

			for (; n > 0 && distance > 0.0f && ((error > 0 && xGood) || (error < 0 && yGood) || (xGood && yGood)); --n)
			{
				if (error > 0)
				{
					x2 += x_inc;
					error -= dy;
					distance -= (decrStrait + (error * 0.004f)) * (1 + (tiles[x2][y2].blockID > 0) * 2);
				}
				else if (error < 0)
				{
					y2 += y_inc;
					error += dx;
					distance -= (decrStrait - (error * 0.004f)) * (1 + (tiles[x2][y2].blockID > 0) * 2);
				}
				else
				{
					x2 += x_inc;
					y2 += y_inc;
					error -= dy;
					error += dx;
					--n;
					distance -= decrDiag * (1 + (tiles[x2][y2].blockID > 0) * 2);
				}

				xGood = (U16)(x2 + x_inc) < TILES_X;
				yGood = (U16)(y2 + y_inc) < TILES_Y;
			}

			distance = Math::Max(distance, 0.0f);

			color += Vector3::ONE * distance * tiles[x][y].lightSource;
			globalColor += Vector3::ONE * distance * tiles[x][y].globalLightSource;
		}
	}

	color = Math::Min(color, Vector3::ONE);
	globalColor = Math::Min(globalColor, Vector3::ONE);
}

void World::BreakBlock(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].blockID)
	{
		Vector2Int chunkPos = pos / 8;

		bool globalLight = !tiles[pos.x][pos.y].wallID;

		TimeSlip::PickupItem(tiles[pos.x][pos.y].blockID, 1);
		if (tiles[pos.x][pos.y].decID > 2) { TimeSlip::PickupItem(tiles[pos.x][pos.y].decID, 1); }

		tiles[pos.x][pos.y].blockID = 0;
		tiles[pos.x][pos.y].decID = 0;
		chunks[chunkPos.x][chunkPos.y].EditBlock(0, pos, pos - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(0, pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[left.x][left.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[left.x][left.y].blockID, left, left - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[left.x][left.y].decID, left, left - chunkPos * 8);

			if (pos.y > 0)
			{
				Vector2Int leftDown = left + Vector2Int::DOWN;
				chunkPos = leftDown / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[leftDown.x][leftDown.y].decID, leftDown, leftDown - chunkPos * 8);
			}

			if (pos.y < TILES_Y - 1)
			{
				Vector2Int leftUp = left + Vector2Int::UP;
				chunkPos = leftUp / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[leftUp.x][leftUp.y].decID, leftUp, leftUp - chunkPos * 8);
			}
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[right.x][right.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[right.x][right.y].blockID, right, right - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[right.x][right.y].decID, right, right - chunkPos * 8);

			if (pos.y > 0)
			{
				Vector2Int rightDown = right + Vector2Int::DOWN;
				chunkPos = rightDown / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[rightDown.x][rightDown.y].decID, rightDown, rightDown - chunkPos * 8);
			}

			if (pos.y < TILES_Y - 1)
			{
				Vector2Int rightUp = right + Vector2Int::UP;
				chunkPos = rightUp / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[rightUp.x][rightUp.y].decID, rightUp, rightUp - chunkPos * 8);
			}
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[down.x][down.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[down.x][down.y].blockID, down, down - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[down.x][down.y].decID, down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[up.x][up.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[up.x][up.y].blockID, up, up - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[up.x][up.y].decID, up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

void World::PlaceBlock(const Vector2Int& pos, U8 id)
{
	if (!tiles[pos.x][pos.y].blockID)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].blockID = id;
		bool globalLight = !tiles[pos.x][pos.y].wallID;
		chunks[chunkPos.x][chunkPos.y].EditBlock(id, pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[left.x][left.y].blockID && !tiles[left.x][left.y].wallID);
			tiles[left.x][left.y].globalLightSource -= globalLight * (tiles[left.x][left.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[left.x][left.y].blockID, left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[right.x][right.y].blockID && !tiles[right.x][right.y].wallID);
			tiles[right.x][right.y].globalLightSource -= globalLight * (tiles[right.x][right.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[right.x][right.y].blockID, right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[down.x][down.y].blockID && !tiles[down.x][down.y].wallID);
			tiles[down.x][down.y].globalLightSource -= globalLight * (tiles[down.x][down.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[down.x][down.y].blockID, down, down - chunkPos * 8);
			if (tiles[down.x][down.y].decID < 2) //It's grass
			{
				tiles[down.x][down.y].decID = 0;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(0, down, down - chunkPos * 8);
			}
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[up.x][up.y].blockID && !tiles[up.x][up.y].wallID);
			tiles[up.x][up.y].globalLightSource -= globalLight * (tiles[up.x][up.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[up.x][up.y].blockID, up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

void World::BreakWall(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].wallID)
	{
		Vector2Int chunkPos = pos / 8;

		bool globalLight = !tiles[pos.x][pos.y].blockID;

		TimeSlip::PickupItem(tiles[pos.x][pos.y].wallID, 1);

		tiles[pos.x][pos.y].wallID = 0;
		chunks[chunkPos.x][chunkPos.y].EditWall(0, pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[left.x][left.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[left.x][left.y].wallID, left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[right.x][right.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[right.x][right.y].wallID, right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[down.x][down.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[down.x][down.y].wallID, down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[up.x][up.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[up.x][up.y].wallID, up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

void World::PlaceWall(const Vector2Int& pos, U8 id)
{
	if (!tiles[pos.x][pos.y].wallID)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].wallID = id;
		bool globalLight = !tiles[pos.x][pos.y].blockID;
		chunks[chunkPos.x][chunkPos.y].EditWall(id, pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[left.x][left.y].blockID && !tiles[left.x][left.y].wallID);
			tiles[left.x][left.y].globalLightSource -= globalLight * (tiles[left.x][left.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[left.x][left.y].wallID, left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[right.x][right.y].blockID && !tiles[right.x][right.y].wallID);
			tiles[right.x][right.y].globalLightSource -= globalLight * (tiles[right.x][right.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[right.x][right.y].wallID, right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[down.x][down.y].blockID && !tiles[down.x][down.y].wallID);
			tiles[down.x][down.y].globalLightSource -= globalLight * (tiles[down.x][down.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[down.x][down.y].wallID, down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[up.x][up.y].blockID && !tiles[up.x][up.y].wallID);
			tiles[up.x][up.y].globalLightSource -= globalLight * (tiles[up.x][up.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(tiles[up.x][up.y].wallID, up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

void World::PlaceLight(const Vector2Int& pos)
{
	if (!tiles[pos.x][pos.y].lightSource)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].lightSource = 1;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

void World::RemoveLight(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].lightSource)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].lightSource = 0;

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes();

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes();
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes();
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes();

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes();
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes();
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes();
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes();
		}
	}
}

F32 World::GenerateWorld()
{
	static const F64 terrainLowFreq = 0.005;
	static const F64 terrainHighFreq = 0.05;
	static const F64 terrainLowAmplitude = 5.0;
	static const F64 terrainHighAmplitude = 25.0;
	static const F64 caveLowFreq = 0.01333333333;
	static const F64 caveHighFreq = 0.02;
	static const F64 caveThresholdMin = 0.01;
	static const F64 caveThresholdMax = 0.04;
	static const F64 oreFreq = 0.07;
	static const F64 oreAmplitude = 0.13;
	static const F64 oreThreshold = 0.1;

	Timer timer;
	timer.Start();

	F32 spawnHeight = 0.0f;

	U16 biomeLengths[BIOME_COUNT] = {};

	U16 lastLength = 0;

	for (U8 i = 0; i < BIOME_COUNT - 1; ++i)
	{
		F64 mod = TILES_X * 0.98 / (F64)BIOME_COUNT - TILES_X * 0.97 / (F64)BIOME_COUNT;
		biomeLengths[i] = (U16)(Math::Mod((F64)Math::RandomF(), mod) + TILES_X * 0.97 / (F64)BIOME_COUNT) + lastLength;
		lastLength = biomeLengths[i];
	}

	biomeLengths[BIOME_COUNT - 1] = TILES_X;

	for (U16 y = 0; y < TILES_Y; ++y)
	{
		U16 length = 0;
		F64 variation = (Math::Simplex1(y * 0.1 + SEED + length) * 5.0);

		for (U8 i = 0; i < BIOME_COUNT - 1; ++i)
		{
			U16 prevLength = length;
			length = biomeLengths[i] + variation;

			for (U16 x = prevLength; x < length; ++x)
			{
				tiles[x][y].biome = i;
			}
		}

		U16 prevLength = length;

		for (U16 x = prevLength; x < TILES_X; ++x)
		{
			tiles[x][y].biome = BIOME_COUNT - 1;
		}
	}

	F64 tempSimplex = Math::Simplex1(SEED);
	U16 prevHeight = (U16)((tempSimplex * terrainHighAmplitude) + (tempSimplex * terrainLowAmplitude) + (TILES_Y * 0.5));

	for (U16 x = 0; x < TILES_X; ++x)
	{
		U16 height = (U16)((Math::Simplex1(x * terrainLowFreq + SEED) * terrainHighAmplitude) +
			(Math::Simplex1(x * terrainHighFreq + SEED) * terrainLowAmplitude) + (TILES_Y * 0.5));

		tiles[x][height].globalLightSource = 1 + (height > prevHeight);
		if (x > 0) { tiles[x - 1][prevHeight].globalLightSource += prevHeight > height; }

		for (U16 y = height; y < TILES_Y; ++y)
		{
			Tile& tile = tiles[x][y];

			bool cave = Math::Abs(Math::Simplex2(x * caveHighFreq + SEED, y * caveHighFreq + SEED) +
				Math::Simplex2(x * caveLowFreq + SEED * 2.0, y * caveLowFreq + SEED * 2.0)) >
				(caveThresholdMax * (height + y) / (F64)height + caveThresholdMin);

			F64 oreNoise = Math::Simplex2(x * oreFreq + SEED * 2.0, y * oreFreq + SEED * 2.0) * oreAmplitude;
			U8 ore = (oreNoise > oreThreshold) * (2 + tile.biome);

			tile.decID = ((y == height) + (y > height) * ore) * cave;
			tile.blockID = ((1 + (y > height + 10)) + biomeTileMods[tile.biome]) * cave;
			tile.wallID = 1 + (y > height + 10) + biomeTileMods[tile.biome];
		}
	}

	Logger::Debug("World Generation Time: {}", timer.CurrentTime());

	return (F32)(U16)((Math::Simplex1((TILES_X >> 1) * terrainLowFreq + SEED) * terrainHighAmplitude) +
		(Math::Simplex1((TILES_X >> 1) * terrainHighFreq + SEED) * terrainLowAmplitude) + (TILES_Y * 0.5)) - 1.5f;
}
