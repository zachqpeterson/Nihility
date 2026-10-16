#pragma once

#include "Defines.hpp"

#include "Core/Containers.hpp"
#include "Components/Registry.hpp"
#include "Physics/Tilemap.hpp"
#include "Physics/Physics.hpp"

enum class EngineState : U8
{
	Editor,
	Playing,
	Paused
};

struct NH_API Scene
{
public:
	virtual ~Scene() = default;

	virtual void OnStart() {}
	virtual void OnUpdate() {}
	virtual void OnStop() {}

	Entity CreateSceneEntity(glm::vec2 position = glm::vec2(0.0f, 0.0f), glm::vec2 scale = glm::vec2(1.0f, 1.0f), F32 rotation = 0.0f)
	{
		Entity entity = Registry::CreateEntity(position, scale, rotation);
		sceneEntities.push_back(entity);
		return entity;
	}

	void AddToScene(const Entity& entity)
	{
		sceneEntities.push_back(entity);
	}

	Entity SpawnPlayer(glm::vec2 spawnPosition)
	{
		Entity playerEntity = Registry::CreateEntity(spawnPosition, { 32.0f, 32.0f }, 0.0f);

		PlayerController& pc = playerEntity.AddComponent<PlayerController>();
		pc.moveSpeed = 150.0f;
		pc.spawnPosition = spawnPosition;

		Rigidbody2D& body = playerEntity.AddComponent<Rigidbody2D>();
		body.velocity = { 0.0f, 0.0f };

		ColliderAABB& aabb = playerEntity.AddComponent<ColliderAABB>();
		aabb.halfExtents = { 16.0f, 16.0f };
		aabb.offset = { 0.0f, 0.0f };

		Hurtbox& hurtbox = playerEntity.AddComponent<Hurtbox>();
		hurtbox.halfExtents = { 10.0f, 10.0f };
		hurtbox.teamId = 0;

		SpriteComponent& sprite = playerEntity.AddComponent<SpriteComponent>();
		sprite.color = { 0.0f, 1.0f, 0.0f, 1.0f };
		sprite.zIndex = 10;

		AddToScene(playerEntity);

		return playerEntity;
	}

	Entity SetupCamera(const Entity& target = {})
	{
		Entity cameraEntity = Registry::CreateEntity();

		cameraEntity.AddComponent<Camera>();

		if (target.Valid())
		{
			CameraTarget& cameraTarget = cameraEntity.AddComponent<CameraTarget>();
			cameraTarget.targetEntity = target.Id();
			cameraTarget.smoothSpeed = 12.0f;
		}

		AddToScene(cameraEntity);

		return cameraEntity;
	}

	void LoadTilemap(const String& filepath)
	{
		Tilemap::Load(filepath);
	}

	void LoadEntities(const String& filepath)
	{
		Registry::LoadState(filepath);
	}

	void ClearScene()
	{
		OnStop();

		for (Entity entity : sceneEntities)
		{
			Registry::DestroyEntity(entity);
		}

		sceneEntities.clear();

		Tilemap::Unload();
	}

private:
	Vector<Entity> sceneEntities;
};

class NH_API SceneManager
{
public:
	static void ChangeScene(std::shared_ptr<Scene> newScene)
	{
		if (activeScene)
		{
			activeScene->ClearScene();
		}

		activeScene = newScene;

		if (activeScene)
		{
			activeScene->OnStart();
		}
	}

	static void Update()
	{
		if (activeScene)
		{
			if (state == EngineState::Playing)
			{
				activeScene->OnUpdate();
			}
		}
	}

	static std::shared_ptr<Scene> GetActiveScene()
	{
		return activeScene;
	}

	static EngineState CurrentState()
	{
		return state;
	}

private:
	static std::shared_ptr<Scene> activeScene;

	static EngineState state;

	friend class Nihility;
	friend class Editor;

	STATIC_CLASS(SceneManager);
};