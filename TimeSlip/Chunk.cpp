#include "Chunk.hpp"

#include <Resources/Resources.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Math/Math.hpp>

Chunk::Chunk() : loaded{ false }, gameObject{ nullptr }
{

}

Chunk::~Chunk()
{

}

void* Chunk::operator new(U64 size) { return Memory::LinearAllocate(sizeof(Chunk)); }

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


}

void Chunk::Unload()
{

}