#include "Tilemap.hpp"

#include "RenderingDefines.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Scene.hpp"
#include "Renderer.hpp"
#include "Platform\Input.hpp"

TilemapComponent::TilemapComponent(U16 width, U16 height) : width{ width }, height{ height }
{
	tiles = Renderer::CreateBuffer(width * height * sizeof(U8), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	legend = Renderer::CreateBuffer(256 * sizeof(U16), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL);
	staging = Renderer::CreateBuffer(width * height * sizeof(U8) + 256 * sizeof(U16), BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_TYPE_CPU_VISIBLE | BUFFER_MEMORY_TYPE_CPU_COHERENT);

	U8* data = (U8*)staging.data;

	for (U32 i = 0; i < width * height; ++i)
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

void TilemapComponent::Update(Scene* scene)
{
	data.eye = scene->GetCamera()->Eye().xy();

	staging.allocationOffset = 0;
}

void TilemapComponent::Load(Scene* scene)
{
	data.eye = scene->GetCamera()->Eye().xy();
	data.width = width;
	data.height = height;

	PushConstant pc = { 0, sizeof(TilemapData), &data };
	ResourceRef<Pipeline> pipeline = Resources::LoadPipeline("pipelines/tilemap.nhpln", 1, &pc);
	pipeline->AddDescriptor({ legend.vkBuffer });
	pipeline->AddDescriptor({ tiles.vkBuffer });
	scene->AddPipeline(pipeline);
}

void TilemapComponent::Cleanup(Scene* scene)
{
	Renderer::DestroyBuffer(tiles);
	Renderer::DestroyBuffer(legend);
	Renderer::DestroyBuffer(staging);
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

void TilemapComponent::ChangeTile(U32 x, U32 y, U8 id)
{
	if (x < 0 || x >= width || y < 0 || y >= height) { return; }

	*((U8*)staging.data + staging.allocationOffset) = id;

	VkBufferCopy copy{};
	copy.dstOffset = x + y * width;
	copy.size = sizeof(U8);
	copy.srcOffset = staging.allocationOffset;

	Renderer::FillBuffer(tiles, staging, 1, &copy);
}