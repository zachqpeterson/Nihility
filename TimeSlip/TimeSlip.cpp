#include "TimeSlip.hpp"

#include "Player.hpp"

#include <Engine.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Scene.hpp>
#include <Resources/Resources.hpp>
#include <Resources/UI.hpp>
#include <Physics/Physics.hpp>
#include <Core/Input.hpp>
#include <Core/Time.hpp>
#include <Math/Math.hpp>

Scene* TimeSlip::mainMenuScene;
Scene* TimeSlip::worldScene;
World* TimeSlip::world;
Player* TimeSlip::player;

GameState TimeSlip::gameState;

WorldSize TimeSlip::smallWorldSize{ WS_SMALL };
WorldSize TimeSlip::mediumWorldSize{ WS_MEDIUM };
WorldSize TimeSlip::largeWorldSize{ WS_LARGE };

bool TimeSlip::Initialize()
{
	gameState = GAME_STATE_MENU;
	mainMenuScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	mainMenuScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);
	RendererFrontend::UseScene(mainMenuScene);

	//UI
	UIElementConfig createWorld{};
	createWorld.position = { 0.375f, 0.375f };
	createWorld.scale = { 0.25f, 0.1f };
	createWorld.color = { 0.0f, 0.0f, 1.0f, 1.0f };
	createWorld.enabled = true;
	createWorld.ignore = false;
	createWorld.scene = mainMenuScene;
	UIElement* worldButton = UI::GeneratePanel(createWorld, true);

	UIElementConfig config{};
	config.position = { 0.1f, -0.2f };
	config.scale = { 0.9f, 0.9f };
	config.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.ignore = true;
	config.scene = mainMenuScene;
	config.parent = worldButton;

	UI::GenerateText(config, "Generate World", 20.0f);

	OnMouse createWorldEvent{};
	createWorldEvent.value = (void*)&smallWorldSize;
	createWorldEvent.callback = CreateWorld;

	worldButton->OnClick = createWorldEvent;

	worldScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
	worldScene->Create(CAMERA_TYPE_ORTHOGRAPHIC);

	return true;
}

void TimeSlip::Shutdown()
{
	if (player) { player->Destroy(); }
	if (world) { world->Destroy(); }
}

bool TimeSlip::Update()
{
	switch (gameState)
	{
	case GAME_STATE_MENU: break;
	case GAME_STATE_GAME:
		player->Update();
		worldScene->GetCamera()->Update();
		world->Update();
		break;
	case GAME_STATE_PAUSE: break;
	default: break;
	}

	return true;
}

void TimeSlip::LoadWorld()
{

}

void TimeSlip::CreateWorld(UIElement* element, ...)
{
	static const F32 camWidth = 3.63636363636f;
	static const F32 camHeight = camWidth * 0.5625f;
	RendererFrontend::UseScene(worldScene);

	Vector2 spawnPoint;
	world = new World(10000000, WS_SMALL, spawnPoint);

	player = new Player(spawnPoint);
	worldScene->GetCamera()->SetPosition({ spawnPoint.x, spawnPoint.y, 10.0f });
	worldScene->GetCamera()->SetTarget(player->gameObject->transform);
	worldScene->GetCamera()->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);

	gameState = GAME_STATE_GAME;
}