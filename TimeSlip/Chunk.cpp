#include "Chunk.hpp"

#include "Tile.h"
#include "World.hpp"

#include <Resources/Resources.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Math/Math.hpp>

World* Chunk::world;

const Vector3 Chunk::VERTEX_POSITIONS[4]{
	Vector3(-0.5f, -0.5f, 0.0f),
	Vector3(0.5f, -0.5f, 0.0f),
	Vector3(0.5f,  0.5f, 0.0f),
	Vector3(-0.5f,  0.5f, 0.0f)
};

const Vector2 Chunk::UV_POSITIONS[4]{
	Vector2(0.0f, 1.0f),
	Vector2(1.0f, 1.0f),
	Vector2(1.0f, 0.0f),
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

		MeshConfig blockConfig;
		blockConfig.MaterialName = "Block.mat"; //TODO: Pre-Load materials
		blockConfig.vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		blockConfig.vertexSize = sizeof(Vertex);
		blockConfig.vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		blockConfig.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		MeshConfig wallConfig;
		wallConfig.MaterialName = "Wall.mat";
		wallConfig.vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		wallConfig.vertexSize = sizeof(Vertex);
		wallConfig.vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		wallConfig.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		MeshConfig decConfig;
		decConfig.MaterialName = "Decoration.mat";
		decConfig.vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		decConfig.vertexSize = sizeof(Vertex);
		decConfig.vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		decConfig.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		MeshConfig liquidConfig;
		liquidConfig.MaterialName = "Liquid.mat";
		liquidConfig.vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		liquidConfig.vertexSize = sizeof(Vertex);
		liquidConfig.vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		liquidConfig.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		Vertex* blockVertices = (Vertex*)blockConfig.vertices;
		Vertex* wallVertices = (Vertex*)wallConfig.vertices;
		Vertex* decVertices = (Vertex*)decConfig.vertices;
		Vertex* liquidVertices = (Vertex*)liquidConfig.vertices;

		U16 index = 0;

		Vector2 cPos = pos * CHUNK_SIZE;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Vector3 worldPos{ (F32)(cPos.x + x), (F32)(cPos.y + y), 0.0f };

				if (tiles[x][y].blockID)
				{
					Vector2 uv = world->BlockUV((Vector2Int)worldPos);

					for (U16 j = 0; j < 4; ++j)
					{
						blockVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], uv + UV_POSITIONS[j], (U32)(tiles[x][y].blockID - 1) };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						blockConfig.indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				if (tiles[x][y].wallID)
				{
					Vector2 uv = world->WallUV((Vector2Int)worldPos);

					for (U16 j = 0; j < 4; ++j)
					{
						wallVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], uv + UV_POSITIONS[j], (U32)(tiles[x][y].wallID - 1) };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						wallConfig.indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				if (tiles[x][y].decID)
				{
					Vector2 uv = world->DecorationUV((Vector2Int)worldPos);

					for (U16 j = 0; j < 4; ++j)
					{
						decVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], uv + UV_POSITIONS[j], (U32)(tiles[x][y].decID - 1) };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						decConfig.indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				if (tiles[x][y].liquidID)
				{
					Vector2 uv = world->LiquidUV((Vector2Int)worldPos);

					for (U16 j = 0; j < 4; ++j)
					{
						liquidVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], uv + UV_POSITIONS[j], (U32)(tiles[x][y].liquidID - 1) };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						liquidConfig.indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				++index;
			}
		}

		model->meshes.Push(Resources::CreateFreeMesh(blockConfig));
		model->meshes.Push(Resources::CreateFreeMesh(wallConfig));
		model->meshes.Push(Resources::CreateFreeMesh(decConfig));
		model->meshes.Push(Resources::CreateFreeMesh(liquidConfig));

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