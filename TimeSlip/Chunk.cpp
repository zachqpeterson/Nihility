#include "Chunk.hpp"

#include "Tile.h"

#include <Resources/Resources.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Math/Math.hpp>

const Vector3 Chunk::VERTEX_POSITIONS[4]{
	Vector3(-0.5f, -0.5f, 0.0f),
	Vector3(0.5f, -0.5f, 0.0f),
	Vector3(0.5f,  0.5f, 0.0f),
	Vector3(-0.5f,  0.5f, 0.0f)
};

const Vector2 Chunk::UV_POSITIONS[4]{
	Vector2(0.0f, 0.125f),
	Vector2(0.16666666666f, 0.125f),
	Vector2(0.16666666666f, 0.0f),
	Vector2(0.0f, 0.0f)
};

const U8 Chunk::INDEX_SEQUENCE[6]{
	0, 1, 2, 2, 3, 0
};

Chunk::Chunk() : loaded{ false }, model{ nullptr }, tiles{ nullptr }
{

}

Chunk::~Chunk()
{
	
}

void Chunk::Destroy()
{
	if (model)
	{
		for (Mesh* m : model->meshes)
		{
			Resources::DestroyFreeMesh(m);
		}

		model->meshes.Destroy();

		Memory::Free(model, sizeof(Model), MEMORY_TAG_GAME);
	}
}

Array<Tile*, CHUNK_SIZE>& Chunk::SetTiles()
{
	return tiles;
}

void Chunk::Load(const Vector2& pos)
{
	static U32 i = 0;

	if (!loaded)
	{
		if (!model) { model = (Model*)Memory::Allocate(sizeof(Model), MEMORY_TAG_GAME); }

		MeshConfig config;
		config.MaterialName = "Tile.mat"; //Pre-Load materials
		config.vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		config.vertexSize = sizeof(Vertex);
		config.vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		config.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		Vertex* vertices = (Vertex*)config.vertices;

		//Mesh data
		U16 index = 0;

		Vector2 cPos = pos * CHUNK_SIZE;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Vector3 worldPos{ (F32)(cPos.x + x), (F32)(cPos.y + y), 0.0f };

				//for (U16 i = 0; i < 4; ++i)
				{
					for (U16 j = 0; j < 4; ++j)
					{
						vertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], UV_POSITIONS[j] * tiles[x][y].blockID, 0 };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						config.indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}

					//++index;
				}

				++index;
			}
		}

		Mesh* mesh = Resources::CreateFreeMesh(config);
		model->meshes.Push(mesh);

		RendererFrontend::DrawModel(model);

		loaded = true;
	}
}

void Chunk::Unload()
{
	if (model)
	{
		RendererFrontend::UndrawModel(model);

		for (Mesh* m : model->meshes)
		{
			Resources::DestroyFreeMesh(m);
		}

		model->meshes.Clear();
	}

	loaded = false;
}