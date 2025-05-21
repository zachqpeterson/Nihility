#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

#include "box2d/box2d.h"

#include "tracy/Tracy.hpp"

U32 Scene::SceneID = 0;

bool Scene::Create(CameraType type)
{
	sceneId = SceneID++;

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

void Scene::Destroy()
{
	vkDeviceWaitIdle(Renderer::device);

	spriteVertexShader.Destroy();
	spriteFragmentShader.Destroy();
	spriteMaterial.Destroy();
}

void Scene::Update()
{
	ZoneScopedN("Scene");
	for (Entity& entity : entities)
	{
		if (entity.bodyId.index != 0)
		{
			b2Transform transform = b2Body_GetTransform(TypePun<b2BodyId>(entity.bodyId));
	
			entity.position.x = transform.p.x;
			entity.position.y = transform.p.y;
			entity.rotation.x = transform.q.s;
			entity.rotation.y = transform.q.c;
		}
	
		if (entity.spriteId != U32_MAX)
		{
			SpriteInstance& instance = spriteInstances[entity.spriteId];
	
			instance.position = entity.position;
			instance.rotation = entity.rotation;
		}
	}

	camera.Update();
	if (spriteInstances.Size())
	{
		spriteMaterial.instanceBuffers[Renderer::frameIndex].UploadVertexData(spriteInstances.Data(), spriteInstances.Size() * sizeof(SpriteInstance), 0, Renderer::renderFinished[Renderer::previousFrame]);
	}
}

void Scene::Render(CommandBuffer commandBuffer) const
{
	if (spriteInstances.Size())
	{
		spriteMaterial.Bind(commandBuffer, (U32)spriteInstances.Size());
	}
}

EntityId Scene::CreateEntity(Vector2 position, Quaternion2 rotation)
{
	Entity entity{};
	entity.position = position;
	entity.rotation = rotation;

	EntityId id{};
	id.entityId = (U32)entities.Size();
	id.sceneId = sceneId;

	entities.Push(entity);

	return id;
}

void Scene::AddSprite(const EntityId& id, const ResourceRef<Texture>& texture, const Vector2& scale, const Vector4& color, const Vector2& textureCoord, const Vector2& textureScale)
{
	if (id.sceneId != sceneId)
	{
		Logger::Error("This Entity Is Not A Part Of This Scene!");
		return;
	}

	if (spriteInstances.Full())
	{
		Logger::Error("Max Instances Reached!");
		return;
	}

	Entity& entity = entities[id.entityId];
	entity.spriteId = (U32)spriteInstances.Size();

	SpriteInstance instance{};
	instance.position = entity.position;
	instance.rotation = entity.rotation;
	instance.scale = scale;
	instance.instColor = color;
	instance.instTexcoord = textureCoord;
	instance.instTexcoordScale = textureScale;
	instance.textureIndex = texture.Handle();

	spriteInstances.Push(instance);
}

void Scene::AddRigidBody(const EntityId& id, BodyType type)
{
	Entity& entity = entities[id.entityId];

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.position.x = entity.position.x;
	bodyDef.position.y = entity.position.y;
	bodyDef.rotation.c = entity.rotation.y;
	bodyDef.rotation.s = entity.rotation.x;
	bodyDef.type = (b2BodyType)type;

	entity.bodyId = TypePun<BodyId>(b2CreateBody(Physics::WorldID(), &bodyDef));
}

void Scene::AddCollider(const EntityId& id, const Vector2& scale)
{
	Entity& entity = entities[id.entityId];

	b2Polygon box = b2MakeBox(scale.x, scale.y);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(TypePun<b2BodyId>(entity.bodyId), &shapeDef, &box);
}
