#include "Chunk.hpp"

#include "Tile.hpp"
#include "World.hpp"

#include <Resources/Resources.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Math/Math.hpp>
#include <Core/Time.hpp>

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
		if (!model) { model = (Model*)Memory::Allocate(sizeof(Model), MEMORY_TAG_GAME); model->meshes.Reserve(4); }

		Vector<MeshConfig> configs{ 4, {} };
		configs[0].MaterialName = "Block.mat";
		configs[0].vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		configs[0].vertexSize = sizeof(Vertex);
		configs[0].vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		configs[0].indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		configs[1].MaterialName = "Wall.mat";
		configs[1].vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		configs[1].vertexSize = sizeof(Vertex);
		configs[1].vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		configs[1].indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		configs[2].MaterialName = "Decoration.mat";
		configs[2].vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		configs[2].vertexSize = sizeof(Vertex);
		configs[2].vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		configs[2].indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		configs[3].MaterialName = "Liquid.mat";
		configs[3].vertices = Memory::Allocate(sizeof(Vertex) * CHUNK_SIZE * CHUNK_SIZE * 4, MEMORY_TAG_RESOURCE);
		configs[3].vertexSize = sizeof(Vertex);
		configs[3].vertexCount = CHUNK_SIZE * CHUNK_SIZE * 4;
		configs[3].indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		Vertex* blockVertices = (Vertex*)configs[0].vertices;
		Vertex* wallVertices = (Vertex*)configs[1].vertices;
		Vertex* decVertices = (Vertex*)configs[2].vertices;
		Vertex* liquidVertices = (Vertex*)configs[3].vertices;

		U16 index = 0;

		Vector2 cPos = pos * CHUNK_SIZE;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Tile& tile = tiles[x][y];
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
						configs[0].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
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
						configs[1].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				if (tiles[x][y].decID)
				{
					Vector2 uv = world->DecorationUV((Vector2Int)worldPos, tiles[x][y].decID);

					for (U16 j = 0; j < 4; ++j)
					{
						decVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], uv + UV_POSITIONS[j], (U32)(tiles[x][y].decID - 1) };
					}

					for (U16 j = 0; j < 6; ++j)
					{
						configs[2].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
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
						configs[3].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					}
				}

				++index;
			}
		}

		/*Timer timer;
		timer.Start();
		Logger::Debug(timer.CurrentTime());*/
		Resources::BatchCreateFreeMeshes(configs, model->meshes);
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