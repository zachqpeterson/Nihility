#pragma once

#include <Defines.hpp>
#include <Containers/Array.hpp>

#define CHUNK_SIZE 8

struct Model;
struct Vector2;
struct Vector3;
struct Tile;

class Chunk
{
public:
	Chunk();
	~Chunk();

	void Destroy();

	void Load(const Vector2& pos);
	void Unload();

private:
	Array<Tile*, CHUNK_SIZE>& SetTiles();

	bool loaded;

	Model* model;
	Array<Tile*, CHUNK_SIZE> tiles;

	static const Vector3 VERTEX_POSITIONS[4];
	static const Vector2 UV_POSITIONS[4];
	static const U8 INDEX_SEQUENCE[6];

	friend class World;
};