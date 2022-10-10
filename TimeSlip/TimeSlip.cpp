#include "TimeSlip.hpp"

#include "Player.hpp"

#include <Engine.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Scene.hpp>
#include <Resources/Resources.hpp>
#include <Physics/Physics.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Math/Math.hpp>

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;
Player* TimeSlip::player;

bool TimeSlip::Initialize()
{
	static const F32 camWidth = 2.4f;
	static const F32 camHeight = 1.35f;

	mainMenuScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	mainMenuScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	//UI

	worldScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	worldScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	RendererFrontend::UseScene(worldScene);

	Vector2 spawnPoint;
	CreateWorld(WS_SMALL, spawnPoint);

	player = new Player(spawnPoint);
	worldScene->GetCamera()->SetPosition({ spawnPoint.x, spawnPoint.y, 10.0f });
	worldScene->GetCamera()->SetTarget(player->gameObject->transform);
	worldScene->GetCamera()->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);

	return true;
}

void TimeSlip::Shutdown()
{
	world->Destroy();
}

bool TimeSlip::Update()
{
	player->Update();
	worldScene->GetCamera()->Update();
	world->Update();

	return true;
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::CreateWorld(WorldSize size, Vector2& spawnPoint)
{
	world = new World(10000000, size, spawnPoint);
}