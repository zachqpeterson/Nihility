#include "CharacterComponent.hpp"

#include "Scene.hpp"

#include "box2d/box2d.h"

Vector<Vector<Character>> Character::components;
bool Character::initialized = false;

bool Character::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Character::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Character> Character::AddTo(const EntityRef& entity)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<Character>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Character Count Reached!");
		return {};
	}

	U32 instanceId = (U32)instances.Size();

	Character character{};

	instances.Push(character);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

bool Character::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<Character>& instances = components[sceneId];

	for (Character& character : instances)
	{
		Entity& entity = entities[character.entityIndex];


	}

	return false;
}

bool Character::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	return false;
}