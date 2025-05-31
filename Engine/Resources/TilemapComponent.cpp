#include "TilemapComponent.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

TilemapData Tilemap::tilemapData;
DescriptorSet Tilemap::tilemapDescriptor;
Material Tilemap::tilemapMaterial;
Shader Tilemap::tilemapVertexShader;
Shader Tilemap::tilemapFragmentShader;
Vector<Vector<Tilemap>> Tilemap::components;
bool Tilemap::initialized = false;

bool Tilemap::Initialize()
{
	if (!initialized)
	{
		initialized = true;

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

		Vector2Int renderSize = Renderer::RenderSize();

		tilemapData.width = 100;
		tilemapData.height = 100;
		tilemapData.tileSize = Vector2::One / (F32)(renderSize.x * 0.01640625);

		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;
	}

	return false;
}

bool Tilemap::Shutdown()
{
	if (initialized)
	{
		initialized = false;

		for (Vector<Tilemap>& instances : components)

		{
			for (Tilemap& tilemap : instances)
			{
				tilemap.tiles.Destroy();
			}
		}

		tilemapVertexShader.Destroy();
		tilemapFragmentShader.Destroy();
		tilemapMaterial.Destroy();
		tilemapDescriptor.Destroy();
	}

	return false;
}

bool Tilemap::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Tilemap>& instances = components[sceneId];

	tilemapData.eye = camera.Eye().xy();

	return false;
}

bool Tilemap::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Tilemap>& instances = components[sceneId];

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

	return false;
}

ComponentRef<Tilemap> Tilemap::AddTo(const EntityRef& entity)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<Tilemap>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Tilemap Instances Reached!");
		return nullptr;
	}

	U32 instanceId = (U32)instances.Size();

	Tilemap tilemap{};
	tilemap.tiles.Create(BufferType::Storage, tilemapData.width * tilemapData.height * sizeof(U16));

	U16* tiles;
	Memory::Allocate(&tiles, tilemapData.width * tilemapData.height);

	for (U32 i = 0; i < tilemapData.width * tilemapData.height; ++i)
	{
		tiles[i] = U16_MAX;
	}

	tilemap.tiles.UploadUniformData(tiles, tilemapData.width * tilemapData.height * sizeof(U16), 0);

	instances.Push(tilemap);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

void Tilemap::SetTile(const ResourceRef<Texture>& texture, Vector2Int position)
{
	U16 handle = texture.Handle();

	tiles.UploadUniformData(&handle, sizeof(U16), (position.x + position.y * tilemapData.width) * sizeof(U16));
}