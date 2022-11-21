#include "Chunk.hpp"

#include "Tile.hpp"
#include "World.hpp"

#include <Resources/Resources.hpp>
#include <Resources/Animations.hpp>
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

Chunk::Chunk() : loaded{ false }, model{ nullptr }, tiles{ nullptr } {}

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

void Chunk::Load(const Vector2Int& pos)
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

		Vector2Int cPos = pos * CHUNK_SIZE;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Vector3 worldPos{ (F32)(cPos.x + x), (F32)(cPos.y + y), 0.0f };
				Tile& tile = tiles[x][y];

				Vector2 blockUV = world->BlockUV((Vector2Int)worldPos);
				Vector2 wallUV = world->WallUV((Vector2Int)worldPos);
				Vector2 decUV = world->DecorationUV((Vector2Int)worldPos, tile.decID);
				Vector2 liquidUV = world->LiquidUV((Vector2Int)worldPos);
				Vector3 color;
				Vector3 globalColor;
				world->TileLight((Vector2Int)worldPos, color, globalColor);

				for (U16 j = 0; j < 4; ++j)
				{
					blockVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], blockUV + UV_POSITIONS[j], color, globalColor, (U32)(tile.blockID - 1) };
					wallVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], wallUV + UV_POSITIONS[j], color, globalColor, (U32)(tile.wallID - 1) };
					decVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], decUV + UV_POSITIONS[j], color, globalColor, (U32)(tile.decID - 1) };
					liquidVertices[index * 4 + j] = Vertex{ worldPos + VERTEX_POSITIONS[j], liquidUV + UV_POSITIONS[j], color, globalColor, (U32)(tile.liquidID - 1) };
				}

				for (U16 j = 0; j < 6; ++j)
				{
					configs[0].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					configs[1].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					configs[2].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
					configs[3].indices[index * 6 + j] = index * 4 + INDEX_SEQUENCE[j];
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

void Chunk::EditBlock(const Vector2Int& worldPos, const Vector2Int& tilePos)
{
	if (loaded)
	{
		Mesh* mesh = model->meshes[0];

		Vector2 uv = world->BlockUV(worldPos);
		Vector3 pos = (Vector3)worldPos;

		Vertex* vertices = (Vertex*)mesh->vertices;

		U32 index = tilePos.x * CHUNK_SIZE + tilePos.y;

		for (U16 j = 0; j < 4; ++j)
		{
			vertices[index * 4 + j].texId = tiles[tilePos.x][tilePos.y].blockID - 1;
			vertices[index * 4 + j].uv = uv + UV_POSITIONS[j];
		}
	}
}

void Chunk::EditWall(const Vector2Int& worldPos, const Vector2Int& tilePos)
{
	if (loaded)
	{
		Mesh* mesh = model->meshes[1];

		Vector2 uv = world->WallUV(worldPos);
		Vector3 pos = (Vector3)worldPos;

		Vertex* vertices = (Vertex*)mesh->vertices;

		U32 index = tilePos.x * CHUNK_SIZE + tilePos.y;

		for (U16 j = 0; j < 4; ++j)
		{
			vertices[index * 4 + j].texId = tiles[tilePos.x][tilePos.y].wallID - 1;
			vertices[index * 4 + j].uv = uv + UV_POSITIONS[j];
		}
	}
}

void Chunk::EditDecoration(const Vector2Int& worldPos, const Vector2Int& tilePos)
{
	if (loaded)
	{
		Mesh* mesh = model->meshes[2];

		Vector2 uv = world->DecorationUV(worldPos, tiles[tilePos.x][tilePos.y].decID);
		Vector3 pos = (Vector3)worldPos;

		Vertex* vertices = (Vertex*)mesh->vertices;

		U32 index = tilePos.x * CHUNK_SIZE + tilePos.y;

		for (U16 j = 0; j < 4; ++j)
		{
			vertices[index * 4 + j].texId = tiles[tilePos.x][tilePos.y].decID - 1;
			vertices[index * 4 + j].uv = uv + UV_POSITIONS[j];
		}
	}
}

void Chunk::EditLiquid(const Vector2Int& worldPos, const Vector2Int& tilePos)
{
	if (loaded)
	{
		Mesh* mesh = model->meshes[3];

		Vector2 uv = world->LiquidUV(worldPos);
		Vector3 pos = (Vector3)worldPos;

		Vertex* vertices = (Vertex*)mesh->vertices;

		U32 index = tilePos.x * CHUNK_SIZE + tilePos.y;

		for (U16 j = 0; j < 4; ++j)
		{
			vertices[index * 4 + j].texId = tiles[tilePos.x][tilePos.y].liquidID - 1;
			vertices[index * 4 + j].uv = uv + UV_POSITIONS[j];
		}
	}
}

void Chunk::AddAnimation(const Vector2Int& worldPos, const Vector2Int& tilePos)
{
	Mesh* mesh = model->meshes[2];
	Vector2 uv = world->DecorationUV(worldPos, tiles[tilePos.x][tilePos.y].decID);
	Vertex* vertices = (Vertex*)mesh->vertices;
	U32 index = tilePos.x * CHUNK_SIZE + tilePos.y;

	for (U16 j = 0; j < 4; ++j)
	{
		vertices[index * 4 + j].texId = tiles[tilePos.x][tilePos.y].decID - 1;
	}

	Animation* anim = Animations::AddAnimation(mesh, index * 4, 12, 5, 4, 5.0f, false);
	anim->direction = true; //TODO: Set using the UV
	Animations::SetAnimation(anim, (U8)uv.y, true, true);
	animations.PushBack(anim);
}

void Chunk::UpdateLighting(const Vector2Int& pos)
{
	if (loaded)
	{
		Mesh* blockMesh = model->meshes[0];
		Vertex* blockVertices = (Vertex*)blockMesh->vertices;
		Mesh* wallMesh = model->meshes[1];
		Vertex* wallVertices = (Vertex*)wallMesh->vertices;
		Mesh* decMesh = model->meshes[2];
		Vertex* decVertices = (Vertex*)decMesh->vertices;
		Mesh* liquidMesh = model->meshes[3];
		Vertex* liquidVertices = (Vertex*)liquidMesh->vertices;

		U16 index = 0;

		for (U16 x = 0; x < CHUNK_SIZE; ++x)
		{
			for (U16 y = 0; y < CHUNK_SIZE; ++y)
			{
				Tile& tile = tiles[x][y];
				if (tile.blockID || tile.wallID)
				{
					Vector3 worldPos{ (F32)(pos.x * CHUNK_SIZE + x), (F32)(pos.y * CHUNK_SIZE + y), 0.0f };
					Vector3 color;
					Vector3 globalColor;
					world->TileLight((Vector2Int)worldPos, color, globalColor);

					for (U16 j = 0; j < 4; ++j)
					{
						blockVertices[index * 4 + j].color = color;
						blockVertices[index * 4 + j].globalColor = globalColor;
						wallVertices[index * 4 + j].color = color;
						wallVertices[index * 4 + j].globalColor = globalColor;
						decVertices[index * 4 + j].color = color;
						decVertices[index * 4 + j].globalColor = globalColor;
						liquidVertices[index * 4 + j].color = color;
						liquidVertices[index * 4 + j].globalColor = globalColor;
					}
				}

				++index;
			}
		}
	}
}

void Chunk::UpdateMeshes(Vector<Mesh*>& meshes)
{
	for (Mesh* mesh : model->meshes)
	{
		meshes.Push(mesh);
	}
}