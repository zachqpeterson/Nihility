#include "Sprite.hpp"

#include "Resources\Scene.hpp"
#include "Resources\Resources.hpp"

struct SpriteInstance
{
	U32 textureIndex;
	U32 entityIndex;
	Vector2 instTexcoord;
	Vector2 texcoordScale;
	Vector4 color;
};

ResourceRef<Material> SpriteComponent::material;
ResourceRef<Mesh> SpriteComponent::mesh;

bool SpriteComponent::Initialize()
{
	MaterialInfo matInfo{};
	matInfo.name = "sprite_material";
	matInfo.effect = Resources::GetMaterialEffect("spriteEffect");
	material = Resources::CreateMaterial(matInfo);

	Vector2 positions[4]{
		{ -5.0f, 5.0f },
		{ 5.0f, 5.0f },
		{ -5.0f, -5.0f },
		{ 5.0f, -5.0f }
	};

	Vector2 texcoords[4]{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f }
	};

	mesh = Resources::CreateMesh("ui_mesh");
	mesh->vertexCount = 4;
	mesh->indicesSize = 0;

	VertexBuffer positionBuffer{};
	positionBuffer.type = VERTEX_TYPE_POSITION;
	positionBuffer.size = (U32)(sizeof(Vector2) * CountOf(positions));
	positionBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&positionBuffer.buffer, CountOf(positions));
	Copy((Vector2*)positionBuffer.buffer, positions, CountOf(positions));

	VertexBuffer texcoordBuffer{};
	texcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
	texcoordBuffer.size = (U32)(sizeof(Vector2) * CountOf(texcoords));
	texcoordBuffer.stride = sizeof(Vector2);
	Memory::AllocateSize(&texcoordBuffer.buffer, CountOf(texcoords));
	Copy((Vector2*)texcoordBuffer.buffer, texcoords, CountOf(texcoords));

	mesh->buffers.Push(positionBuffer);
	mesh->buffers.Push(texcoordBuffer);

	return true;
}

SpriteComponent::SpriteComponent(const Vector4& color, const ResourceRef<Texture>& texture, const Vector4& textureCoords)
{
	static bool i = Initialize();

	meshInstance.material = material;
	meshInstance.mesh = mesh;

	SpriteInstance* instanceData = (SpriteInstance*)meshInstance.instanceData.data;
	if(texture) { instanceData->textureIndex = (U32)texture->Handle(); }
	else { instanceData->textureIndex = U16_MAX; }
	instanceData->instTexcoord = textureCoords.xy();
	instanceData->texcoordScale = textureCoords.zw() - textureCoords.xy();
	instanceData->color = color;
}

void SpriteComponent::Update(Scene* scene, U32 entityID)
{

}

void SpriteComponent::Load(Scene* scene, U32 entityID)
{
	Copy((U32*)meshInstance.instanceData.data + 1, &entityID, 1);

	scene->AddInstance(meshInstance);
}