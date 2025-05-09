#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

bool Scene::Create(CameraType type)
{
	spriteInstances.Reserve(10000);
	camera.Create(type);

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

		{ 2, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, position) },
		{ 3, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, scale) },
		{ 4, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, rotation) },
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

void Scene::Destroy()
{
	spriteVertexShader.Destroy();
	spriteFragmentShader.Destroy();
	spriteMaterial.Destroy();
}

void Scene::Update()
{
	camera.Update();
	if (dirtySprites)
	{
		spriteMaterial.instanceBuffers[Renderer::frameIndex].UploadVertexData(spriteInstances.Data(), spriteInstances.Size() * sizeof(SpriteInstance), 0, Renderer::vertexInputFinished[Renderer::previousFrame]);
	}
}

void Scene::Render(CommandBuffer commandBuffer) const
{
	if (spriteInstances.Size())
	{
		spriteMaterial.Bind(commandBuffer, (U32)spriteInstances.Size());
	}
}

SpriteInstance* Scene::AddSprite(const ResourceRef<Texture>& texture, const Transform& transform, const Vector4& color, const Vector2& textureCoord, const Vector2& textureScale)
{
	if (spriteInstances.Full())
	{
		Logger::Error("Max Instances Reached!");
		return nullptr;
	}

	dirtySprites = true;

	SpriteInstance instance{};
	instance.transform = transform;
	instance.instColor = color;
	instance.instTexcoord = textureCoord;
	instance.instTexcoordScale = textureScale;
	instance.textureIndex = texture.Handle();

	return &spriteInstances.Push(instance);
}
