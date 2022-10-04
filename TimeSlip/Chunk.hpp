#pragma once

#include <Defines.hpp>

#define CHUNK_SIZE 8

struct GameObject2D;
struct Vector2;
struct Vector3;
struct Tile;

class Chunk
{
public:
	Chunk();
	~Chunk();

	void Load(const Vector2& pos);
	void Unload();

private:
	void SetTiles(Tile** tiles);

	bool loaded;

	GameObject2D* gameObject; 
	Tile** tiles;

	static const Vector3 VERTEX_POSITIONS[4];
	static const Vector2 UV_POSITIONS[4];
	static const U8 INDEX_SEQUENCE[6];

	friend class World;
};