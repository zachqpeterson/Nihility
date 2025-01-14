#include "Tilemap.hpp"

#include "Renderer.hpp"
#include "RenderingDefines.hpp"

#include "Resources\Resources.hpp"
#include "Resources\Scene.hpp"
#include "Platform\Input.hpp"

TilemapComponent::TilemapComponent(U16 width, U16 height, Vector2 tileSize) : width(width), height(height), tileSize(tileSize * 8.0f)
{
	tiles = Renderer::CreateBuffer(width * height * sizeof(U8), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	legend = Renderer::CreateBuffer(256 * sizeof(U16), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	staging = Renderer::CreateBuffer(width * height * sizeof(U8) + 256 * sizeof(U16), BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_TYPE_CPU_VISIBLE | BUFFER_MEMORY_TYPE_CPU_COHERENT);

	U8* data = (U8*)staging.data;

	for (U32 i = 0; i < (U32)(width * height); ++i)
	{
		data[i] = U8_MAX;
	}

	VkBufferCopy copy{};
	copy.dstOffset = 0;
	copy.size = width * height * sizeof(U8);
	copy.srcOffset = 0;

	staging.allocationOffset += width * height * sizeof(U8);

	Renderer::FillBuffer(tiles, staging, 1, &copy);
}

//void TilemapComponent::Update(Scene* scene)
//{
//	Vector4 area = Renderer::RenderArea();
//
//	F32 ratio = 1920.0f / area.z;
//
//	data.tileSize = Vector2{ 1.0f / tileSize.x * ratio, 1.0f / tileSize.y * ratio } * scene->GetCamera().Zoom();
//
//	F32 offsetX = (tileSize.x / ratio) * -scene->GetCamera().ZoomVal() * scene->GetCamera().Zoom();
//	F32 offsetY = (tileSize.y / ratio) * scene->GetCamera().ZoomVal() * scene->GetCamera().Zoom() * (1080.0f / 1920.0f);
//	F32 scale = 8.0f * (area.z / 1920.0f) / scene->GetCamera().Zoom();
//	data.eye = scene->GetCamera().Eye().xy() * scale + Vector2{ -area.x , area.y } + Vector2{ offsetX, offsetY };
//
//	staging.allocationOffset = 0;
//}
//
//void TilemapComponent::Load(Scene* scene)
//{
//	Vector4 area = Renderer::RenderArea();
//
//	F32 scale = 8.0f * (area.z / 1920.0f) / scene->GetCamera().Zoom();
//	data.eye = scene->GetCamera().Eye().xy() * scale + Vector2{ -area.x, area.y };
//	data.tileSize = Vector2{ 1.0f / tileSize.x * (1920.0f / area.z), 1.0f / tileSize.y * (1080.0f / area.w) } * scene->GetCamera().Zoom();
//	data.width = width;
//	data.height = height;
//
//	PushConstant pc = { 0, sizeof(TilemapData), &data };
//	ResourceRef<Pipeline> pipeline = Resources::LoadPipeline("pipelines/tilemap.nhpln", 1, &pc);
//	if (!pipeline->DescriptorCount())
//	{
//		pipeline->AddDescriptor({ legend.vkBuffer });
//		pipeline->AddDescriptor({ tiles.vkBuffer });
//	}
//	scene->AddPipeline(pipeline);
//}
//
//void TilemapComponent::Cleanup(Scene* scene)
//{
//	Renderer::DestroyBuffer(tiles);
//	Renderer::DestroyBuffer(legend);
//	Renderer::DestroyBuffer(staging);
//}

Vector2Int TilemapComponent::MouseToTilemap(const Camera& camera) const
{
	Vector4 area = Renderer::RenderArea();
	Vector2 cameraPos = camera.Eye().xy() * 8.0f * (area.z / 1920.0f) * camera.Zoom();
	cameraPos.y = -cameraPos.y;

	return Vector2Int{ (((Input::MousePosition() - area.xy()) * camera.Zoom() + cameraPos) / tileSize) * (1920.0f / area.z) };
}

U8 TilemapComponent::AddTile(const ResourceRef<Texture>& texture)
{
	*(U16*)((U8*)staging.data + staging.allocationOffset) = (U16)texture->Handle();

	VkBufferCopy copy{};
	copy.dstOffset = sizeof(U16) * tileCount;
	copy.size = sizeof(U16);
	copy.srcOffset = staging.allocationOffset;

	Renderer::FillBuffer(legend, staging, 1, &copy);

	staging.allocationOffset += sizeof(U16);

	return tileCount++;
}

void TilemapComponent::ChangeTile(const Vector2Int& pos, U8 id)
{
	if ((U32)pos.x >= width || (U32)pos.y >= height) { return; }

	*((U8*)staging.data + staging.allocationOffset) = id;

	VkBufferCopy copy{};
	copy.dstOffset = pos.x + pos.y * width;
	copy.size = sizeof(U8);
	copy.srcOffset = staging.allocationOffset;

	Renderer::FillBuffer(tiles, staging, 1, &copy);
}