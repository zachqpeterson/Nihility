#include "TilemapComponent.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

TilemapData TilemapComponent::tilemapData;
DescriptorSet TilemapComponent::tilemapDescriptor;
Material TilemapComponent::tilemapMaterial;
Shader TilemapComponent::tilemapVertexShader;
Shader TilemapComponent::tilemapFragmentShader;
Vector<Vector<Tilemap>> TilemapComponent::tilemapInstances;

bool TilemapComponent::Initialize()
{
	DescriptorBinding tilesBinding{
		.type = BindingType::StorageBuffer,
		.stages = (U32)ShaderStage::Fragment,
		.count = 1
	};

	tilemapDescriptor.Create({ tilesBinding }, 0, false);

	VkPushConstantRange pushConstant{};
	pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(TilemapData);

	PipelineLayout tilemapPipelineLayout;

	tilemapPipelineLayout.Create({ tilemapDescriptor, Resources::BindlessTexturesDescriptorSet() }, { pushConstant });

	tilemapVertexShader.Create("shaders/tilemap.vert.spv", ShaderStage::Vertex);
	tilemapFragmentShader.Create("shaders/tilemap.frag.spv", ShaderStage::Fragment);

	Pipeline tilemapPipeline;
	tilemapPipeline.Create(tilemapPipelineLayout, { PolygonMode::Fill }, { tilemapVertexShader, tilemapFragmentShader }, {}, {});
	tilemapMaterial.Create(tilemapPipelineLayout, tilemapPipeline, { tilemapDescriptor, Resources::BindlessTexturesDescriptorSet() },
		{ PushConstant{ &tilemapData, sizeof(TilemapData), 0, VK_SHADER_STAGE_FRAGMENT_BIT } });

	return true;
}

void TilemapComponent::Shutdown()
{
	for (Vector<Tilemap>& instances : tilemapInstances)

	{
		for (Tilemap& tilemap : instances)
		{
			tilemap.tiles.Destroy();
		}
	}

	tilemapVertexShader.Destroy();
	tilemapFragmentShader.Destroy();
	tilemapMaterial.Destroy();
}

void TilemapComponent::Update(U32 sceneId, const Camera& camera, Vector<Entity>& entities)
{
	Vector<Tilemap>& instances = tilemapInstances[sceneId];

	tilemapData.eye = camera.Eye().xy();
}

void TilemapComponent::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	Vector<Tilemap>& instances = tilemapInstances[sceneId];

	for (Tilemap& tilemap : instances)
	{
		VkDescriptorBufferInfo bufferInfo = {
			.buffer = tilemap.tiles,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = tilemapDescriptor,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &bufferInfo,
			.pTexelBufferView = nullptr
		};

		vkUpdateDescriptorSets(Renderer::GetDevice(), 1, &write, 0, nullptr);

		tilemapMaterial.Bind(commandBuffer);
	}
}

void TilemapComponent::AddScene(U32 sceneId)
{
	if (sceneId < tilemapInstances.Size())
	{
		if (tilemapInstances[sceneId].Size())
		{
			Logger::Error("Scene Already Added!");
			return;
		}

		tilemapInstances[sceneId].Reserve(8);
	}
	else if (sceneId == tilemapInstances.Size())
	{
		tilemapInstances.Push({ 8 });
	}
	else
	{
		Logger::Error("Invalid Scene!");
	}
}

void TilemapComponent::RemoveScene(U32 sceneId)
{
	if (sceneId >= tilemapInstances.Size())
	{
		Logger::Error("Invalid Scene!");
		return;
	}

	tilemapInstances[sceneId].Clear();
}

void TilemapComponent::AddComponent(U32 sceneId, U16 width, U16 height, Vector2 tileSize)
{
	if (sceneId >= tilemapInstances.Size())
	{
		Logger::Error("Invalid Scene!");
		return;
	}

	Vector<Tilemap>& instances = tilemapInstances[sceneId];

	if (instances.Full())
	{
		Logger::Error("Max Sprite Instances Reached!");
		return;
	}

	Vector2Int renderSize = Renderer::RenderSize();

	tilemapData.width = width;
	tilemapData.height = height;
	tilemapData.tileSize = tileSize / (renderSize.x * 0.01640625);

	U32 instanceId = (U32)instances.Size();

	Tilemap tilemap{};
	tilemap.tiles.Create(BufferType::Storage, width * height * sizeof(U16));

	U16* tiles;
	Memory::Allocate(&tiles, width * height);

	for (I32 i = 0; i < width * height; ++i)
	{
		tiles[i] = U16_MAX;
	}

	tilemap.tiles.UploadUniformData(tiles, width * height * sizeof(U16), 0);

	instances.Push(tilemap);
}

void TilemapComponent::SetTile(const ResourceRef<Texture>& texture, Vector2Int position)
{
	U16 handle = texture.Handle();

	Tilemap& tilemap = tilemapInstances[0][0];

	tilemap.tiles.UploadUniformData(&handle, sizeof(U16), (position.x + position.y * tilemapData.width) * sizeof(U16));
}