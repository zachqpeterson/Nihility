#include "SpriteComponent.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

Material SpriteComponent::spriteMaterial;
Shader SpriteComponent::spriteVertexShader;
Shader SpriteComponent::spriteFragmentShader;
Vector<Vector<SpriteInstance>> SpriteComponent::spriteInstances;

bool SpriteComponent::Initialize()
{
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
		{ 1, sizeof(SpriteInstance), VK_VERTEX_INPUT_RATE_INSTANCE}
	};

	Vector<VkVertexInputAttributeDescription> attributes = {
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, position) },
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, texcoord) },

		{ 2, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, position) },
		{ 3, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, scale) },
		{ 4, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, rotation) },
		{ 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SpriteInstance, instColor) },
		{ 6, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, instTexcoord) },
		{ 7, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, instTexcoordScale) },
		{ 8, 1, VK_FORMAT_R32_UINT, offsetof(SpriteInstance, textureIndex) },
	};

	Pipeline spritePipeline;
	spritePipeline.Create(spritePipelineLayout, { PolygonMode::Fill }, { spriteVertexShader, spriteFragmentShader }, inputs, attributes);
	spriteMaterial.Create(spritePipelineLayout, spritePipeline, { Resources::DummyDescriptorSet(), Resources::BindlessTexturesDescriptorSet() });

	return true;
}

void SpriteComponent::Shutdown()
{
	spriteVertexShader.Destroy();
	spriteFragmentShader.Destroy();
	spriteMaterial.Destroy();
}

void SpriteComponent::Update(U32 sceneId, const Vector<Entity>& entities)
{
	Vector<SpriteInstance>& instances = spriteInstances[sceneId];

	for (const Entity& entity : entities)
	{
		if (entity.spriteId != U32_MAX)
		{
			SpriteInstance& instance = instances[entity.spriteId];

			instance.position = entity.position;
			instance.rotation = entity.rotation;
		}
	}

	if (instances.Size())
	{
		spriteMaterial.instanceBuffers[Renderer::FrameIndex()].UploadVertexData(instances.Data(), instances.Size() * sizeof(SpriteInstance), 0, Renderer::RenderFinished());
	}
}

void SpriteComponent::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	Vector<SpriteInstance>& instances = spriteInstances[sceneId];

	if (instances.Size())
	{
		spriteMaterial.Bind(commandBuffer, (U32)instances.Size());
	}
}

void SpriteComponent::AddScene(U32 sceneId)
{
	if (sceneId < spriteInstances.Size())
	{
		if (spriteInstances[sceneId].Size())
		{
			Logger::Error("Scene Already Added!");
			return;
		}

		spriteInstances[sceneId].Reserve(10000);
	}
	else if (sceneId == spriteInstances.Size())
	{
		spriteInstances.Push({ 10000 });
	}
	else
	{
		Logger::Error("Invalid Scene!");
	}
}

void SpriteComponent::RemoveScene(U32 sceneId)
{
	if (sceneId >= spriteInstances.Size())
	{
		Logger::Error("Invalid Scene!");
		return;
	}

	spriteInstances[sceneId].Clear();
}

U32 SpriteComponent::AddComponent(U32 sceneId, const ResourceRef<Texture>& texture, const Vector2& scale, const Vector4& color, const Vector2& textureCoord, const Vector2& textureScale)
{
	if (sceneId >= spriteInstances.Size())
	{
		Logger::Error("Invalid Scene!");
		return U32_MAX;
	}

	Vector<SpriteInstance>& instances = spriteInstances[sceneId];

	if (instances.Full())
	{
		Logger::Error("Max Sprite Instances Reached!");
		return U32_MAX;
	}

	U32 instanceId = (U32)instances.Size();

	SpriteInstance instance{};
	instance.scale = scale;
	instance.instColor = color;
	instance.instTexcoord = textureCoord;
	instance.instTexcoordScale = textureScale;
	instance.textureIndex = texture.Handle();

	instances.Push(instance);
	
	return instanceId;
}