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

Chunk::Chunk() : loaded{ false }, gameObject{ nullptr }, tiles{ nullptr }
{

}

Chunk::~Chunk()
{

}

void Chunk::SetTiles(Tile** tiles)
{
	this->tiles = tiles;
}

void Chunk::Load(const Vector2& pos)
{
	static U32 i = 0;

	if (!gameObject)
	{
		String name("Chunk{}", i++);

		Transform2D* transform = new Transform2D();
		transform->Translate({ pos.x * CHUNK_SIZE, pos.y * CHUNK_SIZE });

		MeshConfig config;
		config.name = name;
		config.MaterialName = "Tile.mat";
		config.instanceTextures.Push(Resources::LoadTexture("11dirt_block.bmp"));
		config.vertices.Resize(CHUNK_SIZE * CHUNK_SIZE * 4);
		config.indices.Resize(CHUNK_SIZE * CHUNK_SIZE * 6);

		//Mesh data
		U16 index = 0;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Vector3 worldPos{ (F32)(pos.x + x), (F32)(pos.y + y), 0.0f };

				//for (U16 i = 0; i < 4; ++i)
				{
					for (U16 j = 0; j < 4; ++j)
					{
						config.vertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], UV_POSITIONS[j] * tiles[x][y].blockID };
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

		Mesh* mesh = Resources::CreateMesh(config);
		Vector<Mesh*> meshes(1, mesh);
		Model* model = Resources::CreateModel(name, meshes);

		GameObject2DConfig goConfig{};
		goConfig.name = name;
		goConfig.transform = transform;
		goConfig.model = model;
		gameObject = Resources::CreateGameObject2D(goConfig);
		RendererFrontend::DrawGameObject(gameObject);
	}
	else
	{
		RendererFrontend::CreateMesh(gameObject->model->meshes.Front());
		gameObject->enabled = true;
	}
}

void Chunk::Unload()
{
	gameObject->enabled = false;
	RendererFrontend::DestroyMesh(gameObject->model->meshes.Front());
}