#include "SpriteComponent.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

Material Sprite::spriteMaterial;
Shader Sprite::spriteVertexShader;
Shader Sprite::spriteFragmentShader;
Vector<Vector<Sprite>> Sprite::components;
bool Sprite::initialized = false;

bool Sprite::Initialize()
{
	if (!initialized)
	{
		initialized = true;

		VkPushConstantRange pushConstant{};
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(GlobalPushConstant);

		PipelineLayout spritePipelineLayout;

		spritePipelineLayout.Create({ Resources::DummyDescriptorSet(), Resources::BindlessTexturesDescriptorSet() }, { pushConstant });

		spriteVertexShader.Create("shaders/sprite.vert.spv", ShaderStage::Vertex);
		spriteFragmentShader.Create("shaders/sprite.frag.spv", ShaderStage::Fragment);

		Vector<VkVertexInputBindingDescription> inputs = {
			{ 0, sizeof(SpriteVertex), VK_VERTEX_INPUT_RATE_VERTEX },
			{ 1, sizeof(Sprite), VK_VERTEX_INPUT_RATE_INSTANCE}
		};

		Vector<VkVertexInputAttributeDescription> attributes = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, position) },
			{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, texcoord) },

			{ 2, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Sprite, position) },
			{ 3, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Sprite, scale) },
			{ 4, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Sprite, rotation) },
			{ 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Sprite, instColor) },
			{ 6, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Sprite, instTexcoord) },
			{ 7, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Sprite, instTexcoordScale) },
			{ 8, 1, VK_FORMAT_R32_UINT, offsetof(Sprite, textureIndex) },
			{ 9, 1, VK_FORMAT_R32_UINT, offsetof(Sprite, entityIndex) },
		};

		Pipeline spritePipeline;
		spritePipeline.Create(spritePipelineLayout, { PolygonMode::Fill }, { spriteVertexShader, spriteFragmentShader }, inputs, attributes);
		spriteMaterial.Create(spritePipelineLayout, spritePipeline, { Resources::DummyDescriptorSet(), Resources::BindlessTexturesDescriptorSet() },
			{ PushConstant{ Renderer::GetGlobalPushConstant(), sizeof(GlobalPushConstant), 0, VK_SHADER_STAGE_VERTEX_BIT } });

		SpriteVertex vertices[4] = {
			{ { -1.0f, -1.0f }, { 0.0f, 1.0f } },
			{ { -1.0f,  1.0f }, { 0.0f, 0.0f } },
			{ {  1.0f,  1.0f }, { 1.0f, 0.0f } },
			{ {  1.0f, -1.0f }, { 1.0f, 1.0f } }
		};

		U32 indices[6] = { 0, 1, 2, 2, 3, 0 };

		spriteMaterial.UploadVertices(vertices, sizeof(SpriteVertex) * 4, 0);
		spriteMaterial.UploadIndices(indices, sizeof(U32) * 6, 0);

		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;
	}

	return false;
}

bool Sprite::Shutdown()
{
	if (initialized)
	{
		initialized = false;
		spriteVertexShader.Destroy();
		spriteFragmentShader.Destroy();
		spriteMaterial.Destroy();
	}

	return false;
}

bool Sprite::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Sprite>& instances = components[sceneId];

	for (Sprite& sprite : instances)
	{
		const Entity& entity = entities[sprite.entityIndex];

		sprite.position = entity.position;
		sprite.scale = entity.scale;
		sprite.rotation = entity.rotation;
	}

	if (instances.Size())
	{
		spriteMaterial.UploadInstances(instances.Data(), (U32)(instances.Size() * sizeof(Sprite)), 0);
	}

	return false;
}

bool Sprite::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Sprite>& instances = components[sceneId];

	spriteMaterial.Bind(commandBuffer);

	return false;
}

ComponentRef<Sprite> Sprite::AddTo(const EntityRef& entity, const ResourceRef<Texture>& texture, const Vector4& color, const Vector2& textureCoord, const Vector2& textureScale)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<Sprite>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Sprite Instances Reached!");
		return nullptr;
	}

	U32 instanceId = (U32)instances.Size();

	Sprite instance{};
	instance.instColor = color;
	instance.instTexcoord = textureCoord;
	instance.instTexcoordScale = textureScale;
	instance.textureIndex = texture.Handle();
	instance.entityIndex = entity.EntityId();

	instances.Push(instance);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}