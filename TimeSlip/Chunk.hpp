#pragma once

#include <Defines.hpp>

#define CHUNK_SIZE 8

struct GameObject2D;

class Chunk
{
	Chunk();
	~Chunk();

	void Load(const Vector2& pos);
	void Unload();

	bool loaded;

	GameObject2D* gameObject;
};