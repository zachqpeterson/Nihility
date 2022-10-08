#pragma once

#include <Defines.hpp>

enum WorldSize
{
	WS_TEST = 280,
	WS_SMALL = 2800,
	WS_MEDIUM = 4200,
	WS_LARGE = 5600
};

struct Tile;
struct Vector2;
struct Vector2Int;
class Chunk;

class World
{
public:
	World(I64 seed, WorldSize size);
	~World();
	void Destroy();
	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update();

	Vector2 BlockUV(const Vector2Int& pos);
	Vector2 WallUV(const Vector2Int& pos);
	Vector2 DecorationUV(const Vector2Int& pos, U8 id);
	Vector2 LiquidUV(const Vector2Int& pos);

	F32 GenerateWorld();

private:
	const I64 SEED;
	const U16 TILES_X;
	const U16 TILES_Y;
	const U16 CHUNKS_X;
	const U16 CHUNKS_Y;

	Tile** tiles;
	Chunk** chunks;
};